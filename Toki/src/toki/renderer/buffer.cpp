#include "tkpch.h"
#include "framebuffer.h"
#include "renderer.h"
#include "platform/vulkan/vulkan_buffer.h"

namespace Toki {

    Buffer::Buffer(const BufferConfig& config) : config(config) {}

    Ref<VertexBuffer> VertexBuffer::create(const VertexBufferConfig& config) {
        return createRef<VulkanVertexBuffer>(config);
    }

    Ref<IndexBuffer> IndexBuffer::create(const IndexBufferConfig& config) {
        return createRef<VulkanIndexBuffer>(config);
    }

    Ref<UniformBuffer> UniformBuffer::create(const UniformBufferConfig& config) {
        return createRef<VulkanUniformBuffer>(config);
    }

    Ref<StorageBuffer> StorageBuffer::create(const BufferConfig& config) {
        return createRef<VulkanStorageBuffer>(config);
    }

}