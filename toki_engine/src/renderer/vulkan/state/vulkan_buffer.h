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

    void create(RendererContext* ctx, const Config& config);
    void destroy(RendererContext* ctx);

    void set_data(RendererContext* ctx, u32 size, void* data);

    VkBuffer get_handle() const;
    u32 get_size() const;

private:
    VkBuffer m_handle;
    VkDeviceMemory m_memory;
    VkBufferUsageFlagBits m_usage;
    VkMemoryRequirements m_memory_requirements;
    u32 m_size;
};

}  // namespace toki
