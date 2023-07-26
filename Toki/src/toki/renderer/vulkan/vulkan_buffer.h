#pragma once

#include "tkpch.h"
#include "vulkan_renderer.h"

namespace Toki {

    struct VulkanBufferData {
        VkDeviceSize size;
        void* data;
    };

    struct VulkanBufferSpecification {
        VkDeviceSize size;
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags properties;
    };

    class VulkanBuffer {
    public:
        VulkanBuffer() = default;
        VulkanBuffer(VkDeviceSize size);
        ~VulkanBuffer();

        void setData(VulkanBufferData* data);

        void* mapMemory();
        void unmapMemory();

        VkBuffer getBuffer() { return buffer; }
        VkDeviceMemory getMemory() { return memory; }

        static TkRef<VulkanBuffer> create(VulkanBufferSpecification* spec);
        static TkRef<VulkanBuffer> createStatic(VulkanBufferSpecification* spec, VulkanBufferData* data);
        static void copyBuffer(TkRef<VulkanBuffer> src, TkRef<VulkanBuffer> dst, uint32_t size);

    private:
        void cleanup();

        uint32_t size = 0;
        VkBuffer buffer{};
        VkDeviceMemory memory{};
        bool isMemoryMapped = false;
        void* mappedData = nullptr;
    };

}