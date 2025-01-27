#pragma once

#include <vulkan/vulkan.h>

#include "core/base.h"
#include "core/macros.h"
#include "engine/window.h"
#include "memory/allocators/basic_allocator.h"
#include "renderer/renderer_types.h"

namespace toki {

namespace vulkan_renderer {

#define VK_CHECK(result, message, ...) { TK_ASSERT(result == VK_SUCCESS, message __VA_OPT__(, ) __VA_ARGS__) }

constexpr u32 MAX_ALLOCATED_BUFFERS = 256;
constexpr u32 MAX_ALLOCATED_IMAGES = 256;
constexpr u32 MAX_SWAPCHAIN_COUNT = 1;
constexpr u32 MAX_RENDER_PASS_COUNT = 1;
constexpr u32 MAX_PIPELINE_COUNT = 256;

constexpr u32 MAX_RENDER_PASS_ATTACHMENT_COUNT = 8;

constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

constexpr u32 MAX_DESCRIPTOR_BINDING_COUNT = 8;
constexpr u32 MAX_DESCRIPTOR_SET_COUNT = 2;

constexpr u32 MAX_IN_FLIGHT_COMMAND_BUFFERS = 8;

// Arbitrary sizes
constexpr u64 DEFAULT_STAGING_BUFFER_SIZE = Gigabytes(1);
constexpr u64 DEFAULT_VERTEX_BUFFER_SIZE = Megabytes(500);
constexpr u64 DEFAULT_INDEX_BUFFER_SIZE = Megabytes(200);
constexpr u64 DEFAULT_UNIFORM_BUFFER_SIZE = Megabytes(300);

struct DefaultBufferSize {
    VkBufferUsageFlags usage;
    u64 size;
};

constexpr DefaultBufferSize DEFAULT_BUFFER_SIZES[] = {
    { VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, DEFAULT_VERTEX_BUFFER_SIZE },
    { VK_BUFFER_USAGE_INDEX_BUFFER_BIT, DEFAULT_INDEX_BUFFER_SIZE },
    { VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, DEFAULT_UNIFORM_BUFFER_SIZE },
};

constexpr const char* vulkan_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

constexpr u32 SHADER_STAGE_COUNT = std::to_underlying(ShaderStage::SHADER_STAGE_COUNT);

struct ShaderBinary {
    ShaderStage stage;
    std::vector<u32> binary;
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

struct VulkanBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkBufferUsageFlags usage;
    VkMemoryRequirements memory_requirements;
    u32 memory_property_flags;
};

struct VulkanImage {
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory memory;
    VkImageLayout layout;
    VkFormat format;
};

struct FrameData {
    VkSemaphore present_semaphore;
    VkSemaphore image_available_semaphore;
    VkFence render_fence;

    VkCommandBuffer command_buffer;
    u32 recorded_command_buffer_count;
};

struct Swapchain {
    VkSwapchainKHR swapchain;
    VkSurfaceKHR surface;
    Window* window;
    VkExtent2D extent;

    VkSurfaceFormatKHR surface_format;

    u32 current_frame_index = 0;
    u32 image_count;

    BasicRef<VkImageView> image_views;
    BasicRef<FrameData> frames;

    b8 can_render : 1;
    b8 is_recording_commands : 1;
    b8 waiting_to_present : 1;
    u8 submit_count;
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

struct RenderPass {
    VkRenderPass render_pass;
    BasicRef<VkFramebuffer> framebuffers;
    PackedAttachments attachments;
    u16 attachment_count;
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

struct FreeSection {
    u32 offset;
    u32 size;
};

struct InternalBuffer {
    VulkanBuffer internal_buffer_data;
    BasicRef<FreeSection> free_sections;
    u32 free_section_count;
    u32 offset;
};

}  // namespace vulkan_renderer

}  // namespace toki
