#include "buffer.h"

#include "renderer/vulkan_buffer.h"
#include "toki/core/core.h"

namespace Toki {

uint32_t _Buffer::getSize() const {
    return m_size;
}

Ref<VertexBuffer> VertexBuffer::create(const VertexBufferConfig& config) {
    return createRef<VulkanVertexBuffer>(config);
}

VertexBuffer::VertexBuffer(const VertexBufferConfig& config) : _Buffer(config.size), m_binding(config.binding) {}

uint32_t VertexBuffer::getBinding() const {
    return m_binding;
}

Ref<IndexBuffer> IndexBuffer::create(const IndexBufferConfig& config) {
    return createRef<VulkanIndexBuffer>(config);
}

IndexBuffer::IndexBuffer(const IndexBufferConfig& config) : _Buffer(config.size), m_config(config) {}

uint32_t IndexBuffer::getIndexCount() const {
    return m_config.indexCount;
}

IndexSize IndexBuffer::getIndexSize() const {
    return m_config.indexSize;
}

Ref<UniformBuffer> UniformBuffer::create(const UniformBufferConfig& config) {
    return createRef<VulkanUniformBuffer>(config);
}

UniformBuffer::UniformBuffer(const UniformBufferConfig& config) : m_config(config), _Buffer(config.size) {}

}  // namespace Toki