#include "vulkan_utils.h"

#include "platform.h"
#include "toki/core/assert.h"

#ifdef TK_WINDOW_SYSTEM_GLFW
#include <GLFW/glfw3.h>
#endif

namespace Toki {

VkSurfaceKHR VulkanUtils::createSurface(Ref<VulkanContext> context, Ref<Window> window) {
    VkSurfaceKHR surface;

#ifdef TK_WINDOW_SYSTEM_GLFW
    TK_ASSERT_VK_RESULT(
        glfwCreateWindowSurface(context->instance, (GLFWwindow*) window->getHandle(), context->allocationCallbacks, &surface),
        "Could not create surface"
    );
#endif

    return surface;
}

}  // namespace Toki