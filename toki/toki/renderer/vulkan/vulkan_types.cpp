#include "toki/renderer/private/vulkan/vulkan_types.h"

#include <vulkan/vulkan_core.h>

namespace toki::renderer {

void CommandBuffer::begin(const VulkanState& state) {
	VkResult result = vkResetCommandBuffer(command_buffer, 0);
	TK_ASSERT(result);
	this->state = CommandBufferState::RECORDING;
}

void CommandBuffer::end(const VulkanState& state) {
	VkResult result = vkEndCommandBuffer(command_buffer);
	TK_ASSERT(result);
	this->state = CommandBufferState::EXECUTABLE;
}

void CommandBuffer::reset(const VulkanState& state) {
	VkResult result = vkResetCommandBuffer(command_buffer, 0);
	TK_ASSERT(result);
	this->state = CommandBufferState::INITIAL;
}

void VulkanDevice::cleanup(const VulkanState& state) {
	vkDestroyDevice(logical_device, state.allocation_callbacks);
}

void Swapchain::cleanup(const VulkanState& state) {
	cleanup_image_views(state);

	if (handle != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(state.device.logical_device, handle, state.allocation_callbacks);
	}
}

void Swapchain::cleanup_image_views(const VulkanState& state) {
	if (image_views.size() == 0) {
		return;
	}

	for (u32 i = 0; i < image_views.size(); i++) {
		vkDestroyImageView(state.device.logical_device, image_views[i], state.allocation_callbacks);
	}
}

void WindowState::cleanup(const VulkanState& state) {
	swapchain.cleanup(state);
	vkDestroySurfaceKHR(state.instance, surface, state.allocation_callbacks);
}

void VulkanState::cleanup() {
	device.cleanup(*this);

	vkDestroyInstance(instance, allocation_callbacks);
}

}  // namespace toki::renderer
