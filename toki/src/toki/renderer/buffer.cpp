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

VertexBuffer::VertexBuffer(const VertexBufferConfig& config) : m_binding(config.binding) {}

uint32_t VertexBuffer::getBinding() const {
    return m_binding;
}

Ref<IndexBuffer> IndexBuffer::create(const IndexBufferConfig& config) {
    return createRef<VulkanIndexBuffer>(config);
}

IndexBuffer::IndexBuffer(const IndexBufferConfig& config) : m_indexCount(config.indexCount) {}

uint32_t IndexBuffer::getIndexCount() const {
    return m_indexCount;
}

IndexSize IndexBuffer::getIndexSize() const {
    return m_indexSize;
}

}  // namespace Toki