#pragma once

#include <vector>

#include "toki/core/core.h"
#include "toki/renderer/renderer_types.h"

namespace Toki {

struct BufferConfig {
    uint32_t size = 0;
};

struct VertexBufferConfig : BufferConfig {
    uint32_t binding = 0;
};

struct IndexBufferConfig : BufferConfig {
    uint32_t indexCount = 0;
    IndexSize indexSize;
};

class _Buffer {
public:
    uint32_t getSize() const;

private:
    uint32_t m_size = 0;
};

class VertexBuffer : public _Buffer {
public:
    static Ref<VertexBuffer> create(const VertexBufferConfig& config);

    VertexBuffer() = delete;
    VertexBuffer(const VertexBufferConfig& config);
    virtual ~VertexBuffer() = default;

    virtual void setData(uint32_t size, void* data, uint32_t offset = 0) = 0;

    uint32_t getBinding() const;

private:
    uint32_t m_binding = 0;
};

class IndexBuffer : public _Buffer {
public:
    static Ref<IndexBuffer> create(const IndexBufferConfig& config);

    IndexBuffer() = delete;
    IndexBuffer(const IndexBufferConfig& config);
    virtual ~IndexBuffer() = default;

    virtual void setData(uint32_t size, void* data, uint32_t offset = 0) = 0;

    uint32_t getIndexCount() const;
    IndexSize getIndexSize() const;

private:
    uint32_t m_indexCount = 0;
    IndexSize m_indexSize;
};

}  // namespace Toki
