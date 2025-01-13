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
        u32 usage; // VkImageUsageFlagBits
        u32 memory_properties; // VkMemoryPropertyFlags
    };

    void create(Ref<RendererContext> ctx, const Config& config);
    void destroy(Ref<RendererContext> ctx);

    void resize(Ref<RendererContext> ctx, VkExtent3D extent);
    void transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout);

    void set_data(Ref<RendererContext> ctx, u32 size, void* data);

    static void transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout, VkImage image, VkImageAspectFlags aspect_flags);
    static void transition_layout(VkCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout, VulkanImage& image);

    VkImage get_handle() const;
    VkImageView get_image_view() const;

private:
    VkImage m_handle;
    VkImageView m_imageView;
    VkDeviceMemory m_memory;
    VkFormat m_format;
    VkExtent3D m_extent;
    u32 m_usage;
    u32 m_memoryProperties;
    VkImageAspectFlags m_aspectFlags;
};

}  // namespace toki
