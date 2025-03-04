#include "vulkan_platform.h"

#if defined(TK_PLATFORM_LINUX)

#define GLFW_INCLUDE_VULKAN

#if defined(TK_PLATFORM_LINUX) && defined(TK_WAYLAND)
#define VK_USE_PLATFORM_WAYLAND_KHR
#define GLFW_EXPOSE_NATIVE_WAYLAND
#elif defined(TK_PLATFORM_LINUX) && defined(TK_X11)
#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

#include "platform/glfw_window.h"

namespace toki {

namespace renderer {

VkSurfaceKHR create_surface(VkInstance instance, VkAllocationCallbacks* allocation_callbacks, Window* window) {
    GlfwWindow* w = reinterpret_cast<GlfwWindow*>(window);

    VkSurfaceKHR surface{};
#if defined(TK_WAYLAND)
    VkWaylandSurfaceCreateInfoKHR surface_create_info{ VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR };
    surface_create_info.display = glfwGetWaylandDisplay();
    surface_create_info.surface = glfwGetWaylandWindow(reinterpret_cast<GLFWwindow*>(w->get_handle()));

    VkResult result = vkCreateWaylandSurfaceKHR(instance, &surface_create_info, allocation_callbacks, &surface);
#elif defined(TK_X11)
    VkXcbSurfaceCreateInfoKHR surface_create_info{ VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR };
    surface_create_info.connection = handle->connection;
    surface_create_info.window = glfwGetX11Window(reinterpret_cast<GLFWwindow*>(w->get_handle()));

    VkResult result = vkCreateXcbSurfaceKHR(instance, &createInfo, allocation_callbacks, &surface);
#endif

    VK_CHECK(result, "Could not create window surface");

    return surface;
}

}  // namespace renderer

}  // namespace toki

#endif
