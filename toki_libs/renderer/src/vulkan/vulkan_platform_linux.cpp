#include "vulkan_platform.h"

#if defined(TK_PLATFORM_LINUX)

#if defined(TK_WINDOW_SYSTEM_WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
#elif defined(TK_WINDOW_SYSTEM_X11)
#define VK_USE_PLATFORM_XCB_KHR
#endif

#include <vulkan/vulkan.h>

#include "vulkan_types.h"

namespace toki {

VkSurfaceKHR create_surface(
    VkInstance instance, VkAllocationCallbacks* allocation_callbacks, NativeWindowHandle handle) {
    VkSurfaceKHR surface{};
#if defined(TK_WINDOW_SYSTEM_WAYLAND)
    VkWaylandSurfaceCreateInfoKHR surface_create_info{};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surface_create_info.display = handle.wl_display();
    surface_create_info.surface = handle.wl_surface();

    VkResult result = vkCreateWaylandSurfaceKHR(instance, &surface_create_info, allocation_callbacks, &surface);
#elif defined(TK_WINDOW_SYSTEM_X11)
    VkXcbSurfaceCreateInfoKHR surface_create_info{ VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR };
    surface_create_info.connection = handle->connection;
    surface_create_info.window = glfwGetX11Window(reinterpret_cast<GLFWwindow*>(w->get_handle()));

    VkResult result = vkCreateXcbSurfaceKHR(instance, &createInfo, allocation_callbacks, &surface);
#endif

    VK_CHECK(result, "Could not create window surface");

    return surface;
}

}  // namespace toki

#endif
