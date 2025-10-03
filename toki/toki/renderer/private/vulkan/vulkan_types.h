#pragma once

#include <toki/core/core.h>
#include <toki/platform/window.h>
#include <toki/renderer/private/vulkan/vulkan_commands.h>
#include <toki/renderer/types.h>
#include <vulkan/vulkan.h>

namespace toki::renderer {

struct VulkanState;

constexpr const u32 MAX_FRAMES_IN_FLIGHT = 3;

enum VsyncStatus : u8 {
	VSYNC_STATUS_DISABLED = 0,
	VSYNC_STATUS_ENABLED = 1,
	VSYNC_STATUS_SIZE
};

enum QueueFamilyIndices {
	GRAPHICS_FAMILY_INDEX = 0,
	PRESENT_FAMILY_INDEX = 1,
	QUEUE_FAMILY_INDICIES_SIZE
};

enum CommandBufferState {
	INITIAL = 0,
	RECORDING,
	EXECUTABLE,
	PENDING,
	INVALID
};

struct CommandBuffer {
	VkCommandBuffer command_buffer;
	CommandBufferState state;

	void begin(const VulkanState& state);
	void end(const VulkanState& state);
	void reset(const VulkanState& state);
};

struct VulkanDevice {
	VkDevice logical_device;
	VkPhysicalDevice physical_device;

	u32 indices[QUEUE_FAMILY_INDICIES_SIZE]{};

	VkQueue graphics_queue;
	VkQueue present_queue;

	operator VkDevice() {
		return logical_device;
	}

	operator VkPhysicalDevice() {
		return physical_device;
	}

	void cleanup(const VulkanState& state);
};

struct Image {};

struct ImageViewConfig {
	VkImage image;
	VkFormat format;
};

struct SwapchainConfig {
	u32 window_state_index;
	platform::Window* window;
};

struct Swapchain {
	VkSwapchainKHR handle;
	VkSwapchainKHR old_handle;
	VkExtent2D extent;
	VkSurfaceFormatKHR surface_format;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	toki::DynamicArray<VkImageView> image_views;

	void cleanup(const VulkanState& state);
	void cleanup_image_views(const VulkanState& state);
};

struct VulkanShaderLayout {
	VkPipelineLayout pipeline_layout;
};

struct VulkanShader {
	VkPipeline pipeline;
};

struct VulkanBuffer {
	VkBuffer buffer;
};

struct WindowState {
	VkSurfaceKHR surface;
	Swapchain swapchain;
	VkPresentModeKHR present_modes[VsyncStatus::VSYNC_STATUS_SIZE] = { VK_PRESENT_MODE_FIFO_KHR,
																	   VK_PRESENT_MODE_FIFO_KHR };

	void cleanup(const VulkanState& state);
};

struct VulkanState {
	VkInstance instance;
	VulkanDevice device;
	VkAllocationCallbacks* allocation_callbacks;

	void cleanup();
};

enum VulkanSettingsFlags {
	VSYNC_SUPPORTED = 1 << 0,
	VSYNC_ENABLED = 1 << 1,
};

struct VulkanSettings {
	u64 flags = 0;
};

struct VulkanFrames {
	PersistentStaticArray<VkSemaphore, MAX_FRAMES_IN_FLIGHT> image_available_semaphores;
	PersistentStaticArray<VkSemaphore, MAX_FRAMES_IN_FLIGHT> render_finished_semaphores;
	PersistentStaticArray<VkFence, MAX_FRAMES_IN_FLIGHT> in_flight_fences;
};

struct VulkanResources {
	VkCommandPool command_pool;
	VkCommandPool temporary_command_pool;
	VulkanFrames frames;

	PersistentStaticArray<VulkanCommands, 1> commands;

	PersistentDynamicArray<CommandBuffer> command_buffers;
	PersistentArena<VulkanShader, 16> shaders;
	PersistentArena<VulkanShaderLayout, 4> shader_layouts;
};

}  // namespace toki::renderer
