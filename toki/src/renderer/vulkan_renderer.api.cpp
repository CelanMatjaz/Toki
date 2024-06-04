#include "vulkan_renderer.h"

#include "renderer/renderer_state.h"

namespace Toki {

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

}  // namespace Toki
