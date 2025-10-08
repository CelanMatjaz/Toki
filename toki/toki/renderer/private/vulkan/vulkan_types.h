#pragma once

#include <toki/core/core.h>
#include <toki/platform/window.h>
#include <toki/renderer/frontend/renderer_types.h>
#include <toki/renderer/types.h>
#include <vulkan/vulkan.h>

namespace toki::renderer {

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

enum VulkanSettingsFlags {
	VSYNC_SUPPORTED = 1 << 0,
	VSYNC_ENABLED = 1 << 1,
};

union PresentModes {
	VkPresentModeKHR array[VsyncStatus::VSYNC_STATUS_SIZE] = { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_KHR };
	struct {
		VkPresentModeKHR vsync_disabled;
		VkPresentModeKHR vsync_enabled;
	} modes;
};

struct ImageViewConfig {
	VkImage image;
	VkFormat format;
};

struct MemoryAllocateConfig {
	VkMemoryPropertyFlags memory_property_flags;
	VkMemoryRequirements memory_requirements;
};

struct CommandPoolConfig {};

struct DescriptorPoolConfig {
	Span<VkDescriptorPoolSize> pool_sizes;
	u32 max_sets = 1;
};

struct DescriptorSetConfig {
	u32 count;
	VkDescriptorSetLayout layout;
};

struct VulkanSwapchainConfig {
	platform::Window* window;
};

struct StagingBufferConfig {
	u32 size;
};

struct VulkanSettings {
	u64 flags = 0;
};

template <typename T>
concept CommandBufferFunction = requires(T fn, VkCommandBuffer cmd) { fn(cmd); };

}  // namespace toki::renderer
