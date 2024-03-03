#include "vulkan_utils.h"

#include "platform.h"
#include "toki/core/assert.h"

#ifdef TK_WINDOW_SYSTEM_GLFW
#include <GLFW/glfw3.h>
#endif

namespace Toki {

VkSurfaceKHR VulkanUtils::createSurface(Ref<VulkanContext> context, Ref<Window> window) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;

#ifdef TK_WINDOW_SYSTEM_GLFW
    TK_ASSERT_VK_RESULT(
        glfwCreateWindowSurface(context->instance, (GLFWwindow*) window->getHandle(), context->allocationCallbacks, &surface),
        "Could not create surface"
    );
#endif

    TK_ASSERT(surface != VK_NULL_HANDLE, "Surface should not be VK_NULL_HANDLE");

    std::println("new surface {}", (void*) surface);

    return surface;
}

}  // namespace Toki