#include "vulkan_platform.h"

#if defined(TK_PLATFORM_LINUX)

#if defined(TK_PLATFORM_LINUX) && defined(TK_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#elif defined(TK_PLATFORM_LINUX) && defined(TK_X11)
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

#include "platform/glfw_window.h"
#include "renderer/vulkan/vulkan_backend.h"

namespace toki {

VkSurfaceKHR create_surface(VkInstance instance, VkAllocationCallbacks* allocation_callbacks, Window* window) {
    GlfwWindow* w = reinterpret_cast<GlfwWindow*>(window);

#if defined(TK_PLATFORM_LINUX) && defined(TK_WAYLAND)
    VkWaylandSurfaceCreateInfoKHR surface_create_info{};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surface_create_info.display = glfwGetWaylandDisplay();
    surface_create_info.surface = glfwGetWaylandWindow(reinterpret_cast<GLFWwindow*>(w->get_handle()));

    VkSurfaceKHR surface{};
    VkResult result = vkCreateWaylandSurfaceKHR(instance, &surface_create_info, allocation_callbacks, &surface);
#elif defined(TK_PLATFORM_LINUX) && defined(TK_X11)
    // TODO: add X11 surface creation
#endif

    VK_CHECK(result, "Could not create window surface");

    return surface;
}

}  // namespace toki

#endif
