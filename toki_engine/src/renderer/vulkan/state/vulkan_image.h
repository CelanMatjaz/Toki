#pragma once

#include <vulkan/vulkan.h>

#include "core/core.h"

namespace toki {

struct RendererContext;

class VulkanImage {
public:
    struct Config {
        VkFormat format;
        VkExtent3D extent;
        VkImageUsageFlagBits usage;
        VkMemoryPropertyFlags memory_properties;
    };

    void create(Ref<RendererContext> ctx, const Config& config);
    void destroy(Ref<RendererContext> ctx);

    void resize(Ref<RendererContext> ctx, VkExtent3D extent);
    void transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout);

    static void transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout, VkImage image);

    VkImage get_handle() const;
    VkImageView get_image_view() const;

private:
    VkImage m_handle;
    VkImageView m_imageView;
    VkDeviceMemory m_memory;
    VkFormat m_format;
    VkExtent3D m_extent;
    VkImageUsageFlagBits m_usage;
    VkMemoryPropertyFlags m_memoryProperties;
};

}  // namespace toki
