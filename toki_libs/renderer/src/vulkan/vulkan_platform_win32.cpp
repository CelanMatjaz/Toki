#include "vulkan_platform.h"

#if defined(TK_PLATFORM_WINDOWS)

#include "vulkan_types.h"

#define VK_USE_PLATFORM_WIN32
#include <vulkan/vulkan_win32.h>

namespace toki {

namespace platform {
extern HINSTANCE win32_instance;
}

namespace renderer {

VkSurfaceKHR create_surface(
    VkInstance instance, VkAllocationCallbacks* allocation_callbacks, platform::NativeWindowHandle handle) {
    VkWin32SurfaceCreateInfoKHR surface_create_info{};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hwnd = handle;
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
