#pragma once

#include "vulkan_types.h"

namespace toki {

struct InternalRenderPass {
    VkRenderPass render_pass;
    BasicRef<VkFramebuffer> framebuffers;
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
    BasicRef<VkImageView> image_views;
    VkDeviceMemory memory;
    VkFormat format;
    VkExtent3D extent;
    VkImageAspectFlags aspect_flags;
};

struct InternalShader {
    Pipeline pipelines[1];
    VkShaderStageFlags push_constant_stage_flags;
    u8 pipeline_count;
};

struct InternalWindowData {
    Swapchain swapchain;
};

struct InternalFramebuffer {
    BasicRef<InternalImage> color_image;
    BasicRef<InternalImage> depth_stencil_image;
    b8 has_depth : 1;
    b8 has_stencil : 1;
};

}  // namespace toki
