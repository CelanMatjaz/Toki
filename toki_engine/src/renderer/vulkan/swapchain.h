#pragma once

#include <GLFW/glfw3.h>

#include "renderer_state.h"

namespace toki {

class VulkanRenderer;

struct Swapchain {
    static Ref<Swapchain> create(Ref<RendererContext> ctx, Ref<Window> window);

    void destroy(Ref<RendererContext> ctx);
    void recreate(Ref<RendererContext> ctx);

    VkSwapchainKHR swapchainHandle{};
    VkSurfaceKHR surface{};
    VkSurfaceFormatKHR surfaceFormat{};
    VkPresentModeKHR presentMode{};
    VkExtent2D extent{};
    GLFWwindow* windowHandle{};
    u32 imageCount{};
    VkImageView imageViews[FRAME_COUNT]{};
    VkFramebuffer framebuffers[FRAME_COUNT]{};
};

}  // namespace toki
