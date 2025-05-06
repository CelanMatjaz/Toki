#include "vulkan_platform.h"
#include "vulkan_types.h"

namespace toki {

VkSurfaceKHR vulkan_surface_create(
	VkInstance instance, VkAllocationCallbacks* allocation_callbacks, NativeWindow window) {
	VkSurfaceKHR surface{};
#if defined(TK_WINDOW_SYSTEM_WAYLAND)
	constexpr u32 wl_display = 1;

	VkWaylandSurfaceCreateInfoKHR surface_create_info{};
	surface_create_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	surface_create_info.display = (struct wl_display*) &wl_display;
	surface_create_info.surface = (struct wl_surface*) &window.wl_surface;

	VkResult result = vkCreateWaylandSurfaceKHR(instance, &surface_create_info, allocation_callbacks, &surface);
#elif defined(TK_WINDOW_SYSTEM_X11)
	VkXcbSurfaceCreateInfoKHR surface_create_info{ VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR };
	surface_create_info.connection = handle->connection;
	surface_create_info.window = glfwGetX11Window(reinterpret_cast<GLFWwindow*>(w->get_handle()));

	VkResult result = vkCreateXcbSurfaceKHR(instance, &createInfo, allocation_callbacks, &surface);
#elif defined(TK_WINDOW_SYSTEM_GLFW)
	VkResult result =
		glfwCreateWindowSurface(instance, reinterpret_cast<GLFWwindow*>(window.window), allocation_callbacks, &surface);
#endif

	VK_CHECK(result, "Could not create window surface");

	return surface;
}

}  // namespace toki
