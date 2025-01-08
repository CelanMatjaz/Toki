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

struct Frame {
    VkSemaphore present_semaphore;
    VkSemaphore image_available_semaphore;
    VkFence render_fence;
    vulkan_command_buffer command;
    Vec2i window_dimensions;
};

}  // namespace toki
