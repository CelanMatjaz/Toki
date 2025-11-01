#pragma once

#include <toki/renderer/private/vulkan/vulkan_resources.h>
#include <toki/renderer/private/vulkan/vulkan_types.h>

namespace toki {

struct VulkanState {
	VkInstance instance;
	VkAllocationCallbacks* allocation_callbacks;

	// Device data
	VkDevice logical_device;
	VkPhysicalDevice physical_device;
	u32 indices[QUEUE_FAMILY_INDICIES_SIZE]{};
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties;

	// Settings
	VulkanSettings settings;

	// Swapchain
	VulkanSwapchain swapchain;

	// Frames
	VulkanFrames frames;
	u32 current_frame{};

	// Commands
	VulkanCommandPool command_pool;
	VulkanCommandPool temporary_command_pool;
	// UniquePtr<VulkanCommands> commands;
	// PersistentDynamicArray<CommandBuffer> command_buffers;

	// Resources
	VulkanDescriptorPool descriptor_pool;
	VulkanStagingBuffer staging_buffer;
	PersistentArena<VulkanShaderLayout, 4> shader_layouts;
	PersistentArena<VulkanShader, 16> shaders;
	PersistentArena<VulkanBuffer, 16> buffers;
	PersistentArena<VulkanTexture, 256> textures;
	PersistentArena<VulkanSampler, 4> samplers;
};

}  // namespace toki
