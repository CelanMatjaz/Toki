#if defined(TK_PLATFORM_WINDOWS)

#include "renderer/vulkan/platform/renderer_utils.h"

#include "../../macros.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32
#include <vulkan/vulkan_win32.h>

namespace toki {

VkSurfaceKHR create_surface(Ref<RendererContext> ctx, GLFWwindow* window) {
    VkWin32SurfaceCreateInfoKHR surface_create_info{};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hwnd = glfwGetWin32Window(window);
    surface_create_info.hinstance = GetModuleHandle(nullptr);

    VkSurfaceKHR surface{};
    TK_ASSERT_VK_RESULT(
        vkCreateWin32SurfaceKHR(ctx->instance, &surface_create_info, ctx->allocationCallbacks, &surface),
        "Could not create window surface");

    return surface;
}

}  // namespace toki

#endif
