#pragma once

#include "tkpch.h"
#include "vulkan_renderer.h"

namespace Toki {

    class VulkanBuffer {


    public:
        ~VulkanBuffer() = default;

        void setData(uint32_t size, void* data);
        void cleanup();

        vk::Buffer getBuffer() { return buffer; }
        vk::DeviceMemory getMemory() { return memory; }

        static std::unique_ptr<VulkanBuffer> create(uint32_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
        static std::unique_ptr<VulkanBuffer> createVertexBuffer(uint32_t size, void* data);
        static std::unique_ptr<VulkanBuffer> createIndexBuffer(uint32_t size, void* data);
        static std::unique_ptr<VulkanBuffer> createUniformBuffer(uint32_t size, void* data);

    private:
        VulkanBuffer() = default;
        VulkanBuffer(uint32_t size);

        vk::Buffer buffer;
        vk::DeviceMemory memory;
        uint32_t size;
    };

}