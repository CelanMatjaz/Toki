#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <array>

#include "engine/window.h"
#include "renderer/renderer_types.h"

namespace toki {

#define VK_CHECK(result, message, ...) { TK_ASSERT(result == VK_SUCCESS, message __VA_OPT__(, ) __VA_ARGS__) }

#define FRAME_COUNT 3
#define MAX_FRAMEBUFFER_ATTACHMENTS 8
#define STAGING_BUFFER_SIZE 1024 * 1024 * 1024

inline const std::vector<const char*> vulkan_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

struct queue_family_indices {
    i32 graphics = -1;
    i32 present = -1;
    i32 transfer = -1;
};

struct queues {
    VkQueue graphics{};
    VkQueue present{};
    VkQueue transfer{};
};

struct vulkan_buffer {
    VkBuffer handle;
    VkDeviceMemory memory;
    VkBufferUsageFlagBits usage;
    VkMemoryRequirements memory_requirements;
    u32 size;
};

struct vulkan_image {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView image_view;
    VkFormat format;
    VkExtent3D extent;
    VkImageUsageFlagBits usage;
    VkMemoryPropertyFlags memory_properties;
};

struct vulkan_framebuffer {
    u32 color_render_target_count;
    render_target initial_render_targets[MAX_FRAMEBUFFER_ATTACHMENTS];
    VkRenderingAttachmentInfo color_attachments[MAX_FRAMEBUFFER_ATTACHMENTS - 1];
    // VkRenderingAttachmentInfo depth_attachment;
    // VkRenderingAttachmentInfo stencil_attachment;
    std::array<VkFormat, MAX_FRAMEBUFFER_ATTACHMENTS> render_target_formats;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info;

    vulkan_image images[MAX_FRAMEBUFFER_ATTACHMENTS];
    i32 present_target_index = -1;
};

struct vulkan_graphics_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;
};

enum class vulkan_command_buffer_state {
    UNINITIALIZED,
    READY,
    RECORDING_STARTED,
    RECORDING_STOPPED,
    SUBMITED
};

struct vulkan_command_buffer {
    VkCommandBuffer handle;
    vulkan_command_buffer_state state;
};

struct frame {
    VkSemaphore present_semaphore;
    VkSemaphore render_semaphore;
    VkFence render_fence;
    vulkan_command_buffer command;
    Vec2i window_dimensions;
};

struct vulkan_swapchain {
    VkSwapchainKHR handle;

    GLFWwindow* window;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;

    VkCommandPool command_pool;

    u32 current_image_index;
    u32 image_count;
    VkImage images[FRAME_COUNT];
    VkImageView image_views[FRAME_COUNT];

    u32 current_frame;
    frame frames[FRAME_COUNT];

    b8 can_render = true;
};

}  // namespace toki
