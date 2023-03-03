#pragma once

#include "tkpch.h"
#include "vulkan_renderer.h"

namespace Toki {

    class VulkanBuffer {


    public:
        ~VulkanBuffer() = default;

        void setData(uint32_t size, void* data);
        void cleanup();

        VkBuffer getBuffer() { return buffer; }
        VkDeviceMemory getMemory() { return memory; }

        static std::unique_ptr<VulkanBuffer> create(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        static std::unique_ptr<VulkanBuffer> createVertexBuffer(uint32_t size, void* data);
        static std::unique_ptr<VulkanBuffer> createIndexBuffer(uint32_t size, void* data);
        static std::unique_ptr<VulkanBuffer> createUniformBuffer(uint32_t size, void* data);

    private:
        VulkanBuffer() = default;
        VulkanBuffer(uint32_t size);

        VkBuffer buffer;
        VkDeviceMemory memory;
        uint32_t size;
    };

}