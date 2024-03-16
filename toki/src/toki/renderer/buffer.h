#pragma once

#include <vector>

#include "toki/core/core.h"
#include "toki/renderer/renderer_types.h"

namespace Toki {

struct BufferConfig {
    uint32_t size = 0;
};

class _Buffer {
public:
    _Buffer(uint32_t size) : m_size(size) {}
    uint32_t getSize() const;

protected:
    uint32_t m_size = 0;
};

struct VertexBufferConfig : public BufferConfig {
    uint32_t binding = 0;
};

class VertexBuffer : public _Buffer {
public:
    static Ref<VertexBuffer> create(const VertexBufferConfig& config);

    VertexBuffer() = delete;
    VertexBuffer(const VertexBufferConfig& config);
    virtual ~VertexBuffer() = default;

    virtual void setData(uint32_t size, void* data, uint32_t offset = 0) = 0;
    virtual void* mapMemory(uint32_t size, uint32_t offset) = 0;
    virtual void unmapMemory() = 0;

    uint32_t getBinding() const;

private:
    uint32_t m_binding = 0;
};

struct IndexBufferConfig : public BufferConfig {
    IndexSize indexSize;
};

class IndexBuffer : public _Buffer {
public:
    static Ref<IndexBuffer> create(const IndexBufferConfig& config);

    IndexBuffer() = delete;
    IndexBuffer(const IndexBufferConfig& config);
    virtual ~IndexBuffer() = default;

    virtual void setData(uint32_t size, void* data, uint32_t offset = 0) = 0;
    virtual void* mapMemory(uint32_t size, uint32_t offset) = 0;
    virtual void unmapMemory() = 0;

    IndexSize getIndexSize() const;

private:
    IndexBufferConfig m_config;
};

struct UniformBufferConfig : public BufferConfig {};

class UniformBuffer : public _Buffer {
public:
    static Ref<UniformBuffer> create(const UniformBufferConfig& config);

    UniformBuffer() = delete;
    UniformBuffer(const UniformBufferConfig& config);
    virtual ~UniformBuffer() = default;

    virtual void setData(uint32_t size, void* data, uint32_t offset = 0) = 0;
    virtual void* mapMemory(uint32_t size, uint32_t offset) = 0;
    virtual void unmapMemory() = 0;

private:
    UniformBufferConfig m_config;
};

}  // namespace Toki
