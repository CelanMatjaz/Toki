#pragma once

#include "core/core.h"
#include "cstdint"
#include "renderer/buffer.h"

namespace Toki {

    struct BufferConfig {
        uint32_t size;
        bool isStatic = false;
    };

    class Buffer {
    public:
        Buffer(const BufferConfig& config);
        virtual ~Buffer() = default;

        virtual void setData(uint32_t size, void* data) = 0;

    protected:
        BufferConfig config;
    };

    struct VertexBufferConfig : BufferConfig {
        uint32_t binding = 0;
    };

    class VertexBuffer : public Buffer {
    public:
        VertexBuffer(const VertexBufferConfig& config) : Buffer(config), binding(config.binding) {}
        virtual ~VertexBuffer() = default;
        static Ref<VertexBuffer> create(const VertexBufferConfig& config);

        virtual void bind() = 0;
        virtual void setData(uint32_t size, void* data) = 0;

        uint32_t getBinding() { return binding; }

    protected:
        uint32_t binding;
    };

    struct IndexBufferConfig : BufferConfig {
        uint32_t indexCount = 0;
    };

    class IndexBuffer : public Buffer {
    public:
        IndexBuffer(const IndexBufferConfig& config) : Buffer(config), indexCount(config.indexCount) {}
        virtual ~IndexBuffer() = default;
        static Ref<IndexBuffer> create(const IndexBufferConfig& config);

        virtual void bind() = 0;

        uint32_t getIndexCount() { return indexCount; }

    protected:
        uint32_t indexCount;
    };

    struct UniformBufferConfig : BufferConfig {};

    class UniformBuffer : public Buffer {
    public:
        UniformBuffer(const UniformBufferConfig& config) : Buffer(config) {}
        virtual ~UniformBuffer() = default;
        static Ref<UniformBuffer> create(const UniformBufferConfig& config);
    };

    class StorageBuffer : public Buffer {
    public:
        StorageBuffer(const BufferConfig& config) : Buffer(config) {}
        virtual ~StorageBuffer() = default;
        static Ref<StorageBuffer> create(const BufferConfig& config);
    };











}