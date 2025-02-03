#pragma once

#include <vulkan/vulkan.h>

#include "core/base.h"
#include "core/macros.h"
#include "engine/window.h"
#include "memory/allocators/basic_allocator.h"
#include "memory/allocators/basic_ref.h"
#include "renderer/renderer_types.h"
#include "renderer/shader_uniforms.h"

namespace toki {

namespace renderer {

#define VK_CHECK(result, message, ...) { TK_ASSERT(result == VK_SUCCESS, message __VA_OPT__(, ) __VA_ARGS__) }

constexpr u32 MAX_ALLOCATED_IMAGE_COUNT = 256;
constexpr u32 MAX_ALLOCATED_BUFFER_COUNT = 256;
constexpr u32 MAX_RENDER_PASS_COUNT = 10;
constexpr u32 MAX_SHADER_COUNT = 64;
constexpr u32 MAX_PIPELINE_PER_SHADER_COUNT = 16;
constexpr u32 MAX_SWAPCHAIN_COUNT = 8;

constexpr u32 MAX_RENDER_PASS_ATTACHMENT_COUNT = 8;

constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

constexpr u32 MAX_DESCRIPTOR_BINDING_COUNT = 8;
constexpr u32 MAX_DESCRIPTOR_SET_COUNT = 1;

constexpr u32 MAX_IN_FLIGHT_COMMAND_BUFFERS = 8;

// Arbitrary sizes
constexpr u64 DEFAULT_STAGING_BUFFER_SIZE = Gigabytes(1);

constexpr const char* vulkan_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

constexpr u32 SHADER_STAGE_COUNT = std::to_underlying(ShaderStage::SHADER_STAGE_COUNT);

struct ShaderBinary {
    ShaderStage stage;
    std::vector<u32> binary;
};

struct ShaderModule {
    VkShaderModule shader_module;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
};

struct DescriptorBindings {
    u32 binding_counts[MAX_DESCRIPTOR_SET_COUNT]{};
    VkDescriptorSetLayoutBinding bindings[MAX_DESCRIPTOR_BINDING_COUNT][MAX_DESCRIPTOR_SET_COUNT]{};
};

struct PipelineResources {
    VkPipelineLayout pipeline_layout;
    std::vector<ShaderBinary> binaries;
    DescriptorBindings descriptor_bindings;
    std::vector<VkPushConstantRange> push_constants;
};

struct FrameData {
    VkSemaphore present_semaphore;
    VkFence render_fence;

    VkCommandBuffer command_buffer;
    u32 recorded_command_buffer_count;
};

struct Frames {
    FrameData data[MAX_FRAMES_IN_FLIGHT];
};

struct CommandBuffers {
    u32 count;
    BasicRef<VkCommandBuffer> command_buffers;
};

struct Swapchain {
    VkSwapchainKHR swapchain;
    VkSurfaceKHR surface;
    VkExtent2D extent;
    Window* window;

    VkSurfaceFormatKHR surface_format;
    VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];

    u32 image_index = 0;
    u32 image_count;

    BasicRef<VkImage> images;
    BasicRef<VkImageView> image_views;

    b8 can_render : 1;
    b8 is_recording_commands : 1;
    b8 waiting_to_present : 1;
    u8 submit_count : 4;
    VkPresentModeKHR vsync_disabled_present_mode;
};

static_assert(
    MAX_RENDER_PASS_ATTACHMENT_COUNT == 8, "PackedAttachments union requires MAX_RENDER_PASS_ATTACHMENTS to be 8");
// Used to determine render pass clear values
//
// 00 - no attachment
//
// 01 - present/swapchain attachment
//
// 10 - color attachment
//
// 11 - depth/stencil attachment
union PackedAttachments {
    struct {
        u16 a0 : 2;
        u16 a1 : 2;
        u16 a2 : 2;
        u16 a3 : 2;
        u16 a4 : 2;
        u16 a5 : 2;
        u16 a6 : 2;
        u16 a7 : 2;
    } single;
    u16 combined;
};

struct Pipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    VkPipelineBindPoint bind_point;
};

struct Queue {
    VkQueue handle;
    i32 family_index = -1;
};

struct Limits {
    u32 max_framebuffer_width;
    u32 max_framebuffer_height;

    u16 max_push_constant_size;
    u8 max_color_attachments;
};

struct DeviceProperties {
    VkFormat depth_format;
    VkFormat depth_stencil_format;
};

struct TransitionLayoutConfig {
    VkImageLayout old_layout;
    VkImageLayout new_layout;
    VkPipelineStageFlags src_stage;
    VkPipelineStageFlags dst_stage;
};

}  // namespace renderer

}  // namespace toki
