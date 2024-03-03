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

class VertexBuffer;
class IndexBuffer;

class Buffer {
public:
    static Ref<VertexBuffer> create(const VertexBufferConfig& config);
    static Ref<IndexBuffer> create(const IndexBufferConfig& config);

    Buffer() = delete;
    Buffer(const Buffer& other) = delete;
    Buffer(Buffer&& other) = delete;
    Buffer& operator=(const Buffer& other) = delete;
    Buffer& operator=(const Buffer&& other) = delete;
    virtual ~Buffer() = default;

    virtual void setData(uint32_t size, void* data) = 0;

    uint32_t getSize() const { return m_size; }

protected:
    Buffer(uint32_t size);

    uint32_t m_size = 0;
};

class VertexBuffer : public Buffer {
public:
    VertexBuffer() = delete;
    VertexBuffer(const VertexBufferConfig& config);
    virtual ~VertexBuffer() = default;

    uint32_t getBinding();

private:
    uint32_t m_binding = 0;
};

class IndexBuffer : public Buffer {
public:
    IndexBuffer() = delete;
    IndexBuffer(const IndexBufferConfig& config);
    virtual ~IndexBuffer() = default;

    uint32_t getIndexCount();
    IndexSize getIndexSize();

private:
    uint32_t m_indexCount = 0;
    IndexSize m_indexSize;
};

}  // namespace Toki
