#pragma once

#include <toki/core.h>
#include <vulkan/vulkan.h>

#include "renderer_types.h"

namespace toki {

struct VulkanContext;

#define VK_CHECK(result, message, ...) { TK_ASSERT(result == VK_SUCCESS, message __VA_OPT__(, ) __VA_ARGS__) }

constexpr u32 MAX_SWAPCHAIN_COUNT = TK_MAX_WINDOW_COUNT;

constexpr u32 MAX_FRAMES_IN_FLIGHT = 3;

constexpr u32 MAX_LOADED_BUFFER_COUNT = 128;
constexpr u32 MAX_LOADED_IMAGE_COUNT = 128;
constexpr u32 MAX_LOADED_SHADER_COUNT = 32;
constexpr u32 MAX_LOADED_FRAMEBUFFER_COUNT = 8;

constexpr u64 DEFAULT_STAGING_BUFFER_SIZE = GB(1);
constexpr u32 MAX_IN_FLIGHT_COMMAND_BUFFER_COUNT = 8;

constexpr const char* vulkan_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

struct FrameData {
	VkSemaphore present_semaphore;
	VkFence render_fence;
	VkCommandBuffer command_buffer;
};

struct CommandBuffers {
	u64 used_count = 0;
	StaticArray<VkCommandBuffer, MAX_IN_FLIGHT_COMMAND_BUFFER_COUNT> handles;
};

struct Swapchain {
	VkSwapchainKHR handle;
	u32 image_index;
	VkSurfaceKHR surface;
	VkExtent2D extent;
	VkSurfaceFormatKHR surface_format;
	WeakRef<Window> window;
	VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
	DynamicArray<VkImage> images;
	DynamicArray<VkImageView> image_views;
};

struct Queue {
	VkQueue handle;
	i64 family_index = -1;
};

struct Limits {
	u32 max_framebuffer_width;
	u32 max_framebuffer_height;

	u16 max_push_constant_size;
	u16 max_color_attachments;
};

struct DeviceProperties {
	VkFormat depth_format;
	VkFormat depth_stencil_format;
	VkPresentModeKHR vsync_disabled_present_mode;
};

struct TransitionLayoutConfig {
	VkImageLayout old_layout;
	VkImageLayout new_layout;
	VkPipelineStageFlags src_stage;
	VkPipelineStageFlags dst_stage;
};

struct InternalBuffer {
	VkBuffer handle;
	VkDeviceMemory memory;
	VkMemoryRequirements memory_requirements;
	VkBufferUsageFlags usage;
	u32 memory_property_flags;
	u32 size;
};

struct InternalImage {
	VkImage handle;
	DynamicArray<VkImageView> image_views;
	VkDeviceMemory memory;
	VkFormat format;
	VkExtent3D extent;
	VkImageAspectFlags aspect_flags;
};

struct InternalPipeline {
	VkPipeline handle;
	VkPipelineBindPoint bind_point;
};

struct InternalShader {
	Framebuffer framebuffer;
	DynamicArray<InternalPipeline> pipelines;
	VkShaderStageFlags push_constant_stage_flags;
	VkPipelineLayout pipeline_layout;
	ShaderType type;
};

struct InternalFramebuffer {
	BasicRef<InternalImage> color_image;
	BasicRef<InternalImage> depth_stencil_image;
	ColorFormat image_color_format;
	u32 attachment_count : 6;
	b8 has_depth : 1;
	b8 has_stencil : 1;
};

struct VulkanContext {
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties physical_device_properties;
	VkDevice device;
	Queue graphics_queue, present_queue;
	Limits limits;
	DeviceProperties properties;

	VkAllocationCallbacks* allocation_callbacks;
};

struct VulkanResources {
	InternalBuffer staging_buffer{};
	u64 staging_buffer_offset{};

	// Swapchain swapchain;
	// HandleMap<Swapchain> swapchains;
	DynamicArray<Swapchain> swapchains;
	HandleMap<InternalBuffer> buffers{ 128 };
	HandleMap<InternalImage> images{ 128 };
	HandleMap<InternalShader> shaders{ 32 };
	HandleMap<InternalFramebuffer> framebuffers{ 16 };

	DynamicArray<VkCommandPool> command_pools;
	DynamicArray<VkCommandPool> extra_command_pools;

	CommandBuffers command_buffers;
};

struct VulkanSettings {
	b8 vsync_enabled : 1 {};
	b8 non_vsync_supported : 1 {};
	VkPresentModeKHR vsync_disabled_present_mode : 6 {};
	VkClearColorValue color_clear{};
	VkClearDepthStencilValue depth_stencil_clear{ 1.0f, 0 };
};

}  // namespace toki
