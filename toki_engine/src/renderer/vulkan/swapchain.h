#pragma once

#include <GLFW/glfw3.h>

#include "renderer_state.h"
#include "toki/core/core.h"

namespace toki {

class VulkanRenderer;

struct Swapchain {
    static std::shared_ptr<Swapchain> create(RendererContext* ctx, GLFWwindow* window);

    void destroy(RendererContext* ctx);
    void recreate(RendererContext* ctx);

    VkSwapchainKHR swapchainHandle{};
    VkSurfaceKHR surface{};
    VkSurfaceFormatKHR surfaceFormat{};
    VkPresentModeKHR presentMode{};
    VkExtent2D extent{};
    GLFWwindow* windowHandle{};
    VkImageView imageViews[FRAME_COUNT]{};
};

}  // namespace toki
