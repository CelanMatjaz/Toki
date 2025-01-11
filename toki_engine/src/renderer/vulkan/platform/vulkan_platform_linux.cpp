#include "vulkan_platform.h"

#if defined(TK_PLATFORM_LINUX) && defined(TK_WAYLAND)

#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_types.h"

#define GLFW_INCLUDE_VULKAN

#if defined(TK_PLATFORM_LINUX) && defined(TK_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define VK_USE_PLATFORM_WAYLAND
#elif defined(TK_PLATFORM_LINUX) && defined(TK_X11)
#define GLFW_EXPOSE_NATIVE_X11
#define VK_USE_PLATFORM_X11
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

namespace toki {

VkSurfaceKHR create_surface(Ref<RendererContext> ctx, GLFWwindow* window) {
#if defined(TK_PLATFORM_LINUX) && defined(TK_WAYLAND)
    VkWaylandSurfaceCreateInfoKHR surface_create_info{};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surface_create_info.display = glfwGetWaylandDisplay();
    surface_create_info.surface = glfwGetWaylandWindow(window);

    VkSurfaceKHR surface{};
    VkResult result = vkCreateWaylandSurfaceKHR(ctx->instance, &surface_create_info, ctx->allocation_callbacks, &surface);
#elif defined(TK_PLATFORM_LINUX) && defined(TK_X11)
    // TODO: add X11 surface creation
#endif

    VK_CHECK(result, "Could not create window surface");

    return surface;
}

}  // namespace toki

#endif
