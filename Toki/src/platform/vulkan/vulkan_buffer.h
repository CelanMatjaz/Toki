#pragma once 

#include "renderer/buffer.h"
#include "vulkan/vulkan.h"

namespace Toki {
    class VulkanBuffer {
    public:
        VulkanBuffer(const BufferConfig& config, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        ~VulkanBuffer();

        void setData(uint32_t dataSize, void* data);
        void* readData();

        VkBuffer getBuffer() { return buffer; }
        uint32_t getSize() { return config.size; }

    protected:
        VkBufferUsageFlagBits usage;
        VkMemoryPropertyFlagBits memoryProperties;

        VkBuffer buffer;
        VkDeviceMemory memory;
        bool isMemoryMapped = false;
        void* mappedData = nullptr;

        BufferConfig config;
    };

    class VulkanVertexBuffer : public VertexBuffer, public VulkanBuffer {
    public:
        VulkanVertexBuffer(const VertexBufferConfig& config);
        ~VulkanVertexBuffer() = default;
        virtual void bind() override;
        virtual void setData(uint32_t dataSize, void* data) override;
    };

    class VulkanIndexBuffer : public IndexBuffer, public VulkanBuffer {
    public:
        VulkanIndexBuffer(const IndexBufferConfig& config);
        ~VulkanIndexBuffer() = default;
        virtual void bind() override;
        virtual void setData(uint32_t dataSize, void* data) override;
    };

    class VulkanUniformBuffer : public UniformBuffer, public VulkanBuffer {
    public:
        VulkanUniformBuffer(const UniformBufferConfig& config);
        ~VulkanUniformBuffer() = default;
        virtual void setData(uint32_t dataSize, void* data) override;
    };

    class VulkanStorageBuffer : public StorageBuffer, public VulkanBuffer {
    public:
        VulkanStorageBuffer(const BufferConfig& config);
        ~VulkanStorageBuffer() = default;
        virtual void setData(uint32_t dataSize, void* data) override;
    };

}