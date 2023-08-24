#include "tkpch.h"
#include "renderer_command.h"
#include "vulkan/vulkan.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/vulkan/vulkan_buffer.h"
#include "platform/vulkan/vulkan_shader.h"
#include "unordered_map"

namespace Toki {

    void RendererCommand::setViewport(const glm::vec2& position, const glm::vec2& extent) {
        VkViewport viewport{};
        viewport.x = position.x;
        viewport.y = position.y;
        viewport.width = extent.x;
        viewport.height = extent.y;
        vkCmdSetViewport(VulkanRenderer::commandBuffer(), 0, 1, &viewport);
    }

    void RendererCommand::setScissor(const glm::ivec2& position, const glm::ivec2& extent) {
        VkRect2D scissor{};
        scissor.offset = { position.x, position.y };
        scissor.extent = { (uint32_t) extent.x, (uint32_t) extent.y };
        vkCmdSetScissor(VulkanRenderer::commandBuffer(), 0, 1, &scissor);
    }

    void RendererCommand::draw(Ref<VertexBuffer> vertexBuffer) {
        vertexBuffer->bind();
        vkCmdDraw(VulkanRenderer::commandBuffer(), ((VulkanVertexBuffer*) vertexBuffer.get())->getBinding(), 1, 0, 0);
    }

    void RendererCommand::draw(Ref<VertexBuffer> vertexBuffer, uint32_t verteciesCount) {
        vertexBuffer->bind();
        vkCmdDraw(VulkanRenderer::commandBuffer(), verteciesCount, 1, 0, 0);
    }

    void RendererCommand::draw(Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer) {
        vertexBuffer->bind();
        indexBuffer->bind();
        vkCmdDrawIndexed(VulkanRenderer::commandBuffer(), indexBuffer->getIndexCount(), 1, 0, 0, 0);
    }

    void RendererCommand::drawInstanced(std::vector<Ref<VertexBuffer>> vertexBuffers, Ref<IndexBuffer> indexBuffer, uint32_t instanceCount) {
        std::unordered_map<uint32_t, std::vector<VkBuffer>> bufferMap;

        for (const auto& vertexBuffer : vertexBuffers) {
            uint32_t binding = vertexBuffer->getBinding();
            if (!bufferMap.contains(binding))
                bufferMap[binding] = {};
            bufferMap[binding].emplace_back(((VulkanVertexBuffer*) vertexBuffer.get())->getBuffer());
        }

        VkCommandBuffer commandBuffer = VulkanRenderer::commandBuffer();

        VkDeviceSize offsets[] = { 0 };

        for (const auto& [binding, buffers] : bufferMap) {
            vkCmdBindVertexBuffers(commandBuffer, binding, 1, buffers.data(), offsets);
        }

        indexBuffer->bind();

        vkCmdDrawIndexed(commandBuffer, indexBuffer->getIndexCount(), instanceCount, 0, 0, 0);
    }

    void RendererCommand::setConstant(Ref<Shader> shader, ShaderStage stage, uint32_t dataSize, const void* data) {
        vkCmdPushConstants(VulkanRenderer::commandBuffer(), ((VulkanShader*) shader.get())->getPipelineLayout(), VulkanShader::mapShaderStage(stage), 0, dataSize, data);
    }

    void RendererCommand::setUniform(Ref<Shader> shader, Ref<UniformBuffer> uniformBuffer, ShaderStage stage, uint32_t binding, uint32_t set) {
        VulkanShader* vulkanShader = (VulkanShader*) shader.get();
        VulkanUniformBuffer* vulkanUniform = (VulkanUniformBuffer*) uniformBuffer.get();

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = vulkanUniform->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = vulkanUniform->getSize();

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.pNext = nullptr;
        descriptorWrite.dstSet = vulkanShader->getDescriptorSet(stage, set);
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(VulkanRenderer::device(), 1, &descriptorWrite, 0, nullptr);

        auto sets = vulkanShader->getDescriptorSets(stage);

        vkCmdBindDescriptorSets(VulkanRenderer::commandBuffer(), vulkanShader->getPipelineBindPoint(), vulkanShader->getPipelineLayout(), 0, sets.size(), sets.data(), 0, nullptr);
    }

}