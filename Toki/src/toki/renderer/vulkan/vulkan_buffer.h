#pragma once

#include "tkpch.h"
#include "vulkan_renderer.h"

namespace Toki {

    class VulkanBuffer {
    public:
        ~VulkanBuffer();

        void setData(uint32_t size, void* data);
        void cleanup();

        VkBuffer getBuffer() { return buffer; }
        VkDeviceMemory getMemory() { return memory; }

        void* mapMemory(uint32_t size);
        void unmapMemory();

        static std::shared_ptr<VulkanBuffer> create(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        static std::shared_ptr<VulkanBuffer> createVertexBuffer(uint32_t size, void* data = nullptr);
        static std::shared_ptr<VulkanBuffer> createIndexBuffer(uint32_t size, void* data = nullptr);
        static std::shared_ptr<VulkanBuffer> createUniformBuffer(uint32_t size, void* data = nullptr);

    private:
        VulkanBuffer() = default;
        VulkanBuffer(uint32_t size);

        VkBuffer buffer;
        VkDeviceMemory memory;
        uint32_t size;

        bool isMemoryMapped = false;
    };

}