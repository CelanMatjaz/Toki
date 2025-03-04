#include "vulkan_platform.h"

#if defined(TK_PLATFORM_WINDOWS)

#include "platform/glfw_window.h"
#include "renderer/vulkan/vulkan_types.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32
#include <vulkan/vulkan_win32.h>

namespace toki {

namespace renderer {

VkSurfaceKHR create_surface(VkInstance instance, VkAllocationCallbacks* allocation_callbacks, Window* window) {
    GlfwWindow* w = reinterpret_cast<GlfwWindow*>(window);

    VkWin32SurfaceCreateInfoKHR surface_create_info{};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hwnd = glfwGetWin32Window(reinterpret_cast<GLFWwindow*>(w->get_handle()));
    surface_create_info.hinstance = GetModuleHandle(nullptr);

    VkSurfaceKHR surface{};
    VK_CHECK(
        vkCreateWin32SurfaceKHR(instance, &surface_create_info, allocation_callbacks, &surface),
        "Could not create window surface");

    return surface;
}

}  // namespace renderer

}  // namespace toki

#endif
