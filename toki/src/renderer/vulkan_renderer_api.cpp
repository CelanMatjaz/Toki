#include <vulkan/vulkan.h>

#include "renderer/command_pool.h"
#include "renderer/renderer_state.h"
#include "toki/core/assert.h"
#include "vulkan_renderer.h"

namespace Toki {

static CommandBuffer& getCommandBuffer() {
    auto& commandBuffer = s_commandPools[s_currentFrameIndex].getCommandBuffer();
    commandBuffer.m_didRecordCommands = true;
    return commandBuffer;
}

// API implementations

void VulkanRenderer::setViewport(const Region2D& region) {
    VkViewport viewport{};
    viewport.x = region.position.x;
    viewport.y = region.position.y;
    viewport.width = region.extent.x;
    viewport.height = region.extent.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 0.0f;

    vkCmdSetViewport(getCommandBuffer(), 0, 1, &viewport);
}

void VulkanRenderer::resetViewport() {
    auto swapchainExtent = s_swapchainMap.begin()->second->m_extent;
    setViewport({ { 0, 0 }, { swapchainExtent.width, swapchainExtent.height } });
}

void VulkanRenderer::setScissor(const Region2D& region) {
    VkRect2D scissor{};
    scissor.offset.x = region.position.x;
    scissor.offset.y = region.position.y;
    scissor.extent.width = region.extent.x;
    scissor.extent.height = region.extent.y;

    vkCmdSetScissor(getCommandBuffer(), 0, 1, &scissor);
}

void VulkanRenderer::resetScissor() {
    auto swapchainExtent = s_swapchainMap.begin()->second->m_extent;
    setScissor({ { 0, 0 }, { swapchainExtent.width, swapchainExtent.height } });
}

void VulkanRenderer::setLineWidth(float width) {
    vkCmdSetLineWidth(getCommandBuffer(), width);
}

Handle VulkanRenderer::createFramebuffer(const FramebufferConfig& config) {
    Handle handle;
    TK_ASSERT(s_swapchainMap.size() == 1, "Only one swapchain supported");
    s_framebufferMap.emplace(handle, createRef<Framebuffer>(config.attachments, config.extent, s_swapchainMap.begin()->first));
    return handle;
}

void VulkanRenderer::destroyFramebuffer(Handle handle) {
    bool contains = s_framebufferMap.contains(handle);
    TK_ASSERT(contains, "Handle used for destroying a framebuffer is not valid");

    if (contains) {
        s_framebufferMap.erase(handle);
    }
}

void VulkanRenderer::bindFramebuffer(Handle handle) {
    TK_ASSERT(s_framebufferMap.contains(handle), "Invalid framebuffer handle provided: Framebuffer does not exist");
    TK_ASSERT(s_currentlyBoundFramebuffer.get() == nullptr, "Cannot bind a framebuffer while one is already bound");

    const Ref<Framebuffer> framebuffer = s_framebufferMap[handle];
    s_currentlyBoundFramebuffer = framebuffer;

    std::vector<VkClearValue> clearValues(framebuffer->m_attachments.size());

    for (uint32_t i = 0; i < framebuffer->m_attachments.size(); ++i) {
        switch (framebuffer->m_attachments[i].colorFormat) {
            case ColorFormat::R8:
            case ColorFormat::RG8:
            case ColorFormat::RGBA8:
                clearValues[i].color = s_globalClearValues.colorClear;
                break;

            case ColorFormat::Depth:
                clearValues[i].depthStencil.depth = s_globalClearValues.depthClear;
                break;

            case ColorFormat::Stencil:
                clearValues[i].depthStencil.stencil = s_globalClearValues.stencilClear;
                break;

            case ColorFormat::DepthStencil:
                clearValues[i].depthStencil.depth = s_globalClearValues.depthClear;
                clearValues[i].depthStencil.stencil = s_globalClearValues.stencilClear;
                break;
        }
    }

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = *s_renderPassMap[framebuffer->m_renderPassHandle];
    renderPassBeginInfo.framebuffer = *framebuffer;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = { framebuffer->m_extent.width, framebuffer->m_extent.height };
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(getCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderer::unbindFramebuffer() {
    vkCmdEndRenderPass(getCommandBuffer());
    s_currentlyBoundFramebuffer.reset();
}

Handle VulkanRenderer::createShader(const ShaderConfig& config) {
    Handle handle;
    s_pipelineMap.emplace(handle, createRef<Pipeline>(config));
    return handle;
}

void VulkanRenderer::destroyShader(Handle handle) {
    bool contains = s_pipelineMap.contains(handle);

    TK_ASSERT(contains, "Deleting shader: Provided shader handle not valid")

    if (contains) {
        s_pipelineMap.erase(handle);
    }
}

void VulkanRenderer::bindShader(Handle handle) {
    TK_ASSERT(s_pipelineMap.contains(handle), "Invalid shader handle provided: Shader does not exist");
    auto pipeline = s_pipelineMap[handle];
    s_currentlyBoundPipeline = pipeline;
    vkCmdBindPipeline(getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
}

Handle VulkanRenderer::createBuffer(const BufferConfig& config) {
    return createBuffer(config.type, config.size);
}

Handle VulkanRenderer::createBuffer(BufferType bufferType, uint32_t size) {
    Handle handle;

    VkBufferUsageFlags usageBits = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;

    switch (bufferType) {
        case BufferType::VertexBuffer:
            usageBits = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;

        case BufferType::IndexBuffer:
            usageBits = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;

        case BufferType::UniformBuffer:
            usageBits = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;

        default:
            std::unreachable();
    }

    s_bufferMap.emplace(
        handle, createRef<Buffer>(size, usageBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, bufferType));

    return handle;
}

void VulkanRenderer::destroyBuffer(Handle handle) {
    TK_ASSERT(s_bufferMap.contains(handle), "Buffer with provided handle does not exist");
    s_bufferMap.erase(handle);
}

void VulkanRenderer::setBufferData(Handle handle, uint32_t size, uint32_t offset, void* data) {
    TK_ASSERT(s_bufferMap.contains(handle), "Buffer with provided handle does not exist");
    s_bufferMap[handle]->setData(size, offset, data);
}

void VulkanRenderer::bindIndexBuffer(Handle handle, uint32_t offset) {
    TK_ASSERT(s_bufferMap.contains(handle), "Trying to bind an index buffer that does not exist");
    auto buffer = s_bufferMap[handle];
    TK_ASSERT(buffer->m_type == BufferType::IndexBuffer, "Trying to bind a buffer that is not an index buffer");
    vkCmdBindIndexBuffer(getCommandBuffer(), buffer->m_buffer, offset, VK_INDEX_TYPE_UINT32);
}

void VulkanRenderer::bindVertexBuffers(const std::vector<VertexBufferBinding>& bufferBindings) {
    std::vector<VkBuffer> buffers(bufferBindings.size());
    std::vector<VkDeviceSize> offsets(bufferBindings.size(), 0);

    for (uint32_t i = 0; i < bufferBindings.size(); ++i) {
        TK_ASSERT(s_bufferMap.contains(bufferBindings[i].handle), "Trying to bind a vertex buffer that does not exist");
        buffers[i] = s_bufferMap[bufferBindings[i].handle]->m_buffer;
        offsets[i] = bufferBindings[i].offset;
    }

    vkCmdBindVertexBuffers(getCommandBuffer(), bufferBindings[0].binding, bufferBindings.size(), buffers.data(), offsets.data());
}

Handle VulkanRenderer::createTexture(ColorFormat format, std::filesystem::path path) {
    Handle handle;
    s_textureMap.emplace(handle, createRef<Texture>(format, path));
    return handle;
}

Handle VulkanRenderer::createTexture(ColorFormat format, uint32_t width, uint32_t height, uint32_t layers) {
    Handle handle;
    s_textureMap.emplace(handle, createRef<Texture>(format, width, height, layers));
    return handle;
}

Handle VulkanRenderer::createTexture(const TextureConfig& config) {
    Handle handle;
    s_textureMap.emplace(handle, createRef<Texture>(config.format, config.size.x, config.size.y, config.size.z));
    return handle;
}

void VulkanRenderer::destroyTexture(Handle handle) {
    TK_ASSERT(s_textureMap.contains(handle), "Texture with provided handle does not exist");
    s_textureMap.erase(handle);
}

Handle VulkanRenderer::createSampler() {
    Handle handle;
    s_samplerMap.emplace(handle, createRef<Sampler>());
    return handle;
}

void VulkanRenderer::destroySampler(Handle handle) {
    TK_ASSERT(s_samplerMap.contains(handle), "Sampler with provided handle does not exist");
    s_samplerMap.erase(handle);
}

void VulkanRenderer::uploadGeometry(Ref<Geometry> geometry) {}

void VulkanRenderer::setColorClear(const Color& c) {
    s_globalClearValues.colorClear = { { c.r, c.g, c.b, c.a } };
}

void VulkanRenderer::setDepthClear(float clear) {
    s_globalClearValues.depthClear = clear;
}

void VulkanRenderer::setStencilClear(uint32_t clear) {
    s_globalClearValues.stencilClear = clear;
}

void VulkanRenderer::setTexture(Handle handle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex) {
    TK_ASSERT(s_textureMap.contains(handle), "Texture with provided handle does not exist");

    auto texture = s_textureMap[handle];

    VkDescriptorImageInfo descriptorImageInfo{};
    descriptorImageInfo.imageView = texture->m_imageView;
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImageInfo.sampler = VK_NULL_HANDLE;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = s_currentlyBoundPipeline->m_descriptorSets[setIndex];
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writeDescriptorSet.pImageInfo = &descriptorImageInfo;
    writeDescriptorSet.dstArrayElement = arrayIndex;
    writeDescriptorSet.dstBinding = binding;

    vkUpdateDescriptorSets(context.device, 1, &writeDescriptorSet, 0, nullptr);
}
void VulkanRenderer::setSampler(Handle handle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex) {
    TK_ASSERT(s_samplerMap.contains(handle), "Sampler with provided handle does not exist");

    auto sampler = s_samplerMap[handle];

    VkDescriptorImageInfo descriptorImageInfo{};
    descriptorImageInfo.sampler = sampler->m_sampler;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = s_currentlyBoundPipeline->m_descriptorSets[setIndex];
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writeDescriptorSet.pImageInfo = &descriptorImageInfo;
    writeDescriptorSet.dstArrayElement = arrayIndex;
    writeDescriptorSet.dstBinding = binding;

    vkUpdateDescriptorSets(context.device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanRenderer::setUniformBuffer(Handle handle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex) {
    TK_ASSERT(s_bufferMap.contains(handle), "Buffer with provided handle does not exist");
    auto buffer = s_bufferMap[handle];
    TK_ASSERT(buffer->m_type == BufferType::UniformBuffer, "Provided buffer was not created as a uniform buffer");

    VkDescriptorBufferInfo descriptorBufferInfo{};
    descriptorBufferInfo.buffer = buffer->m_buffer;
    descriptorBufferInfo.range = buffer->m_size;
    descriptorBufferInfo.offset = 0;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = s_currentlyBoundPipeline->m_descriptorSets[setIndex];
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
    writeDescriptorSet.dstArrayElement = arrayIndex;
    writeDescriptorSet.dstBinding = binding;

    vkUpdateDescriptorSets(context.device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanRenderer::setTextureWithSampler(Handle textureHandle, Handle samplerHandle, uint32_t setIndex, uint32_t binding, uint32_t arrayIndex) {
    TK_ASSERT(s_textureMap.contains(textureHandle), "Texture with provided handle does not exist");
    TK_ASSERT(s_samplerMap.contains(samplerHandle), "Sampler with provided handle does not exist");

    auto texture = s_textureMap[textureHandle];
    auto sampler = s_samplerMap[samplerHandle];

    VkDescriptorImageInfo descriptorImageInfo{};
    descriptorImageInfo.imageView = texture->m_imageView;
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImageInfo.sampler = sampler->m_sampler;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = s_currentlyBoundPipeline->m_descriptorSets[setIndex];
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.pImageInfo = &descriptorImageInfo;
    writeDescriptorSet.dstArrayElement = arrayIndex;
    writeDescriptorSet.dstBinding = binding;

    vkUpdateDescriptorSets(context.device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanRenderer::bindUniforms(uint32_t firstSet, uint32_t setCount) {
    TK_ASSERT(s_currentlyBoundPipeline.get(), "There is no shader currently bound");
    vkCmdBindDescriptorSets(
        getCommandBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        s_currentlyBoundPipeline->m_pipelineLayout,
        firstSet,
        setCount,
        s_currentlyBoundPipeline->m_descriptorSets.data(),
        0,
        nullptr);
}

void VulkanRenderer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    vkCmdDraw(getCommandBuffer(), vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRenderer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) {
    vkCmdDrawIndexed(getCommandBuffer(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

}  // namespace Toki
