#pragma once

#include <toki/core.h>
#include <vulkan/vulkan.h>

#if defined(TK_WINDOW_SYSTEM_GLFW)
	#include <GLFW/glfw3.h>
#endif

namespace toki {

VkSurfaceKHR vulkan_surface_create(
	VkInstance instance, VkAllocationCallbacks* allocation_callbacks, NativeWindow window);

}  // namespace toki
