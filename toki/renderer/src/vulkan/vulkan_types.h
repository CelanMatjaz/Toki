#pragma once

#include <toki/core.h>
#include <vulkan/vulkan.h>

#include "renderer_types.h"

namespace toki {

#define VK_CHECK(result, message, ...) { TK_ASSERT(result == VK_SUCCESS, message __VA_OPT__(, ) __VA_ARGS__) }

constexpr u32 MAX_SWAPCHAIN_COUNT = TK_MAX_WINDOW_COUNT;

constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

constexpr u32 MAX_LOADED_BUFFER_COUNT = 128;
constexpr u32 MAX_LOADED_IMAGE_COUNT = 128;
constexpr u32 MAX_LOADED_SHADER_COUNT = 32;
constexpr u32 MAX_LOADED_FRAMEBUFFER_COUNT = 8;

constexpr u64 DEFAULT_STAGING_BUFFER_SIZE = GB(1);
constexpr u32 MAX_IN_FLIGHT_COMMAND_BUFFER_COUNT = 8;

constexpr const char* vulkan_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

struct PipelineResources {
	VkPipelineLayout pipeline_layout;
	VkPushConstantRange* push_constants;
	VkDescriptorSetLayout* descriptor_set_layouts;
	u32 push_constant_count;
	u32 descriptor_layout_count;
};

struct FrameData {
	VkSemaphore present_semaphore;
	VkFence render_fence;
	VkCommandBuffer command_buffer;
};

struct CommandBuffers {
	u64 used_count = 0;
	StaticArray<VkCommandBuffer, MAX_IN_FLIGHT_COMMAND_BUFFER_COUNT> handles;
};

struct Surface {
	VkSurfaceKHR handle;
	VkExtent2D extent;
	VkSurfaceFormatKHR format;
	WeakRef<Window> window;
};

struct _Swapchain {
	VkSwapchainKHR handle;
	DynamicArray<VkImage> images;
	DynamicArray<VkImageView> image_views;

	Surface surface;

	VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
	u32 image_index;
};

struct Swapchain {
	VkSwapchainKHR swapchain;
	VkSurfaceKHR surface;
	WeakRef<Window> window_handle;

	VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];

	u32 image_index;
	u32 image_count;

	DynamicArray<VkImage> images;
	DynamicArray<VkImageView> image_views;

	b8 can_render : 1;
	b8 is_recording_commands : 1;
	b8 waiting_to_present : 1;
	u8 submit_count : 4;
	VkPresentModeKHR vsync_disabled_present_mode;
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

struct InternalBuffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkBufferUsageFlags usage;
	VkMemoryRequirements memory_requirements;
	u32 memory_property_flags;
	u32 size;
};

struct InternalImage {
	VkImage image;
	DynamicArray<VkImageView> image_views;
	VkDeviceMemory memory;
	VkFormat format;
	VkExtent3D extent;
	VkImageAspectFlags aspect_flags;
};

struct InternalPipeline {
	VkPipeline pipeline;
	VkPipelineBindPoint bind_point;
};

struct InternalShader {
	Handle framebuffer_handle;
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

}  // namespace toki
