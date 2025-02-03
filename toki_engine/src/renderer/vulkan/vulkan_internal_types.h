#pragma once

#include <cmath>

#include "memory/allocators/basic_ref.h"
#include "renderer/vulkan/vulkan_types.h"

namespace toki {

namespace renderer {

struct InternalRenderPass {
    VkRenderPass render_pass;
    BasicRef<VkFramebuffer> framebuffers;
    PackedAttachments attachments;
    u16 attachment_count;
};

struct InternalBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkBufferUsageFlags usage;
    VkMemoryRequirements memory_requirements;
    u32 memory_property_flags;
};

struct InternalImage {
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory memory;
    // VkImageLayout layout;
    VkFormat format;
    VkImageAspectFlags aspect_flags;
};

struct InternalShader {
    Pipeline pipelines[1];
    u8 pipeline_count;
    u8 layout_counts[MAX_DESCRIPTOR_SET_COUNT];
    VkDescriptorSetLayout descriptor_set_layouts[MAX_DESCRIPTOR_BINDING_COUNT][MAX_DESCRIPTOR_SET_COUNT]
                                                [MAX_FRAMES_IN_FLIGHT];
};

struct InternalWindowData {
    Window* window;
    Swapchain swapchain;
};

struct InternalFramebuffer {
    BasicRef<InternalImage> images;
    BasicRef<InternalImage> depth_stencil_image;
    BasicRef<VkFormat> formats;
    b8 has_present_attachment : 1;
    b8 has_depth : 1;
    b8 has_stencil : 1;
    b8 swapchain_index : 4;
};

}  // namespace renderer

}  // namespace toki
