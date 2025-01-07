#pragma once

#include "renderer/vulkan/vulkan_types.h"

namespace toki {

struct RendererContext;

class VulkanBuffer {
public:
    struct Config {
        u32 size;
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags memory_properties;
    };

    void create(Ref<RendererContext> ctx, const Config& config);
    void destroy(Ref<RendererContext> ctx);

private:
    VkBuffer m_handle;
    VkDeviceMemory m_memory;
    VkBufferUsageFlagBits m_usage;
    VkMemoryRequirements m_memory_requirements;
    u32 m_size;
};

}  // namespace toki
