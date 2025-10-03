#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <toki/renderer/frontend/renderer_types.h>
#include <toki/renderer/private/vulkan/vulkan_backend.h>
#include <toki/runtime/runtime.h>

namespace toki::renderer {

void VulkanBackend::create_swapchain(const SwapchainConfig& config) {
	WindowState& window_state = m_windowStates[config.window_state_index];
	VkSurfaceKHR surface = window_state.surface = create_surface(config.window);

	VkSurfaceCapabilitiesKHR surface_capabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_state.device, surface, &surface_capabilities);
	window_state.swapchain.surface_capabilities = surface_capabilities;

	query_present_modes(window_state);
	query_surface_formats(window_state);
	query_surface_extent(window_state, surface_capabilities);

	recreate_swapchain(window_state);
}

void VulkanBackend::recreate_swapchain(WindowState& window_state) {
	window_state.swapchain.cleanup_image_views(m_state);

	VkSwapchainCreateInfoKHR swapchain_create_info{};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = window_state.surface;
	swapchain_create_info.minImageCount = window_state.swapchain.surface_capabilities.minImageCount;
	swapchain_create_info.imageFormat = window_state.swapchain.surface_format.format;
	swapchain_create_info.imageColorSpace = window_state.swapchain.surface_format.colorSpace;
	swapchain_create_info.imageExtent = window_state.swapchain.extent;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_create_info.oldSwapchain = window_state.swapchain.old_handle;
	swapchain_create_info.preTransform = window_state.swapchain.surface_capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.presentMode =
		window_state.present_modes[!!static_cast<b8>(m_settings.flags & VSYNC_ENABLED & VSYNC_SUPPORTED)];

	if (m_state.device.indices[GRAPHICS_FAMILY_INDEX] == m_state.device.indices[PRESENT_FAMILY_INDEX]) {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = m_state.device.indices;
	} else {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices = nullptr;
	}

	VkResult result = vkCreateSwapchainKHR(
		m_state.device, &swapchain_create_info, m_state.allocation_callbacks, &window_state.swapchain.handle);
	TK_ASSERT(result == VK_SUCCESS);

	if (window_state.swapchain.old_handle != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(m_state.device, window_state.swapchain.old_handle, m_state.allocation_callbacks);
	}

	window_state.swapchain.old_handle = window_state.swapchain.handle;

	u32 swapchain_image_count{};
	vkGetSwapchainImagesKHR(m_state.device, window_state.swapchain.handle, &swapchain_image_count, nullptr);
	TempDynamicArray<VkImage> swapchain_images(swapchain_image_count);
	vkGetSwapchainImagesKHR(
		m_state.device, window_state.swapchain.handle, &swapchain_image_count, swapchain_images.data());
}

VkSurfaceKHR VulkanBackend::create_surface(platform::Window* window) {
	VkSurfaceKHR surface;
	VkResult result = VK_RESULT_MAX_ENUM;

#if defined(TK_WINDOW_SYSTEM_GLFW)
	result = glfwCreateWindowSurface(
		m_state.instance,
		reinterpret_cast<GLFWwindow*>(window->native_handle()),
		m_state.allocation_callbacks,
		&surface);
#else
	#error "Implement platform specific surface creation"
#endif

	TK_ASSERT(result == VK_SUCCESS);

	return surface;
}

void VulkanBackend::query_present_modes(WindowState& window_state) {
	u32 present_mode_count{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_state.device, window_state.surface, &present_mode_count, nullptr);
	TempDynamicArray<VkPresentModeKHR> present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		m_state.device, window_state.surface, &present_mode_count, present_modes.data());

	const VkPresentModeKHR present_modes_without_vsync_priorities[2]{ VK_PRESENT_MODE_MAILBOX_KHR,
																	  VK_PRESENT_MODE_IMMEDIATE_KHR };

	const VkPresentModeKHR present_modes_with_vsync_priorities[2]{ VK_PRESENT_MODE_FIFO_KHR,
																   VK_PRESENT_MODE_FIFO_RELAXED_KHR };

	u8 present_mode_indicies[VsyncStatus::VSYNC_STATUS_SIZE] = { static_cast<u8>(-1), static_cast<u8>(-1) };

	for (u32 i = 0; i < present_modes.size(); i++) {
		for (u32 j = 0; j < CARRAY_SIZE(present_modes_with_vsync_priorities); j++) {
			if (present_modes[i] == present_modes_with_vsync_priorities[j] &&
				present_mode_indicies[VSYNC_STATUS_ENABLED] > j) {
				present_mode_indicies[VSYNC_STATUS_ENABLED] = j;
				break;
			}
		}

		for (u32 j = 0; j < CARRAY_SIZE(present_modes_without_vsync_priorities); j++) {
			if (present_modes[i] == present_modes_without_vsync_priorities[j] &&
				present_mode_indicies[VSYNC_STATUS_DISABLED] > j) {
				present_mode_indicies[VSYNC_STATUS_DISABLED] = j;
				break;
			}
		}
	}

	// Check if vsync present mode is supported
	if (present_mode_indicies[VSYNC_STATUS_ENABLED] != static_cast<u8>(-1)) {
		window_state.present_modes[VSYNC_STATUS_ENABLED] =
			present_modes_with_vsync_priorities[present_mode_indicies[VSYNC_STATUS_ENABLED]];
		m_settings.flags |= VulkanSettingsFlags::VSYNC_SUPPORTED;
	} else {
		m_settings.flags &= ~VulkanSettingsFlags::VSYNC_SUPPORTED;
	}

	window_state.present_modes[VSYNC_STATUS_DISABLED] =
		present_modes_without_vsync_priorities[present_mode_indicies[VSYNC_STATUS_DISABLED]];
}

void VulkanBackend::query_surface_formats(WindowState& window_state) {
	u32 surface_format_count{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_state.device, window_state.surface, &surface_format_count, nullptr);
	TempDynamicArray<VkSurfaceFormatKHR> surface_formats(surface_format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		m_state.device, window_state.surface, &surface_format_count, surface_formats.data());

	for (u32 i = 0; i < surface_formats.size(); ++i) {
		if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			window_state.swapchain.surface_format = surface_formats[i];
			return;
		}
	}

	window_state.swapchain.surface_format = surface_formats[0];
}

void VulkanBackend::query_surface_extent(WindowState& window_state, VkSurfaceCapabilitiesKHR surface_capabilities) {
	if (surface_capabilities.currentExtent.width != static_cast<u32>(-1)) {
		window_state.swapchain.extent = surface_capabilities.currentExtent;
		return;
	}

	auto calculated_extent = convert_to<VkExtent2D>(runtime::Engine::get()->get_window(0)->get_dimensions());
	calculated_extent.width = toki::clamp(
		calculated_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
	calculated_extent.height = toki::clamp(
		calculated_extent.height,
		surface_capabilities.minImageExtent.height,
		surface_capabilities.maxImageExtent.height);

	window_state.swapchain.extent = calculated_extent;
	return;
}

}  // namespace toki::renderer
