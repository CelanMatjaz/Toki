#pragma once

#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_types.h"

namespace Toki {

struct VulkanBufferConfig {
    uint32_t size;
    VkBufferUsageFlagBits usage;
    VkMemoryPropertyFlags properties;
};

class VulkanBuffer {
public:
    static Ref<VulkanBuffer> create(const VulkanContext* context, const VulkanBufferConfig& config);
    static Ref<VulkanBuffer> create(const VulkanContext* context, uint32_t size, void* data, bool isStatic = true);

    VulkanBuffer() = delete;
    VulkanBuffer(const VulkanContext* context, const VulkanBufferConfig& config);
    VulkanBuffer(const VulkanContext* context, uint32_t size, void* data, bool isStatic = true);
    ~VulkanBuffer();

    void setData(uint32_t size, void* data);
    void* mapMemory();
    void unmapMemory();

private:
    void create();
    void destroy();

    const VulkanContext* context;
    VulkanBufferConfig config;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    bool isMemoryMapped = false;
    void* mappedMemory = nullptr;
};

}  // namespace Toki
