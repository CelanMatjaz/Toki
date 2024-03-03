#include "buffer.h"

namespace Toki {

Ref<VertexBuffer> Buffer::create(const VertexBufferConfig& config) {
    return nullptr;
}

Ref<IndexBuffer> Buffer::create(const IndexBufferConfig& config) {
    return nullptr;
}

Buffer::Buffer(uint32_t size) : m_size(size) {}

VertexBuffer::VertexBuffer(const VertexBufferConfig& config) : Buffer(config.size), m_binding(config.binding) {}

uint32_t VertexBuffer::getBinding() {
    return m_binding;
}

IndexBuffer::IndexBuffer(const IndexBufferConfig& config) : Buffer(config.size), m_indexCount(config.indexCount) {}

uint32_t IndexBuffer::getIndexCount() {
    return m_indexCount;
}

IndexSize IndexBuffer::getIndexSize() {
    return m_indexSize;
}

}  // namespace Toki