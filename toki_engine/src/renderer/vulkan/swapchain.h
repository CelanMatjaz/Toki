#pragma once

#include <GLFW/glfw3.h>

#include "renderer/vulkan/data/image_view.h"
#include "renderer_state.h"

namespace toki {

class VulkanRenderer;

struct Swapchain {
    static Swapchain create(Ref<RendererContext> ctx, Ref<Window> window);
    static void cleanup(Ref<RendererContext> ctx, Swapchain& swapchain);
    static void recreate(Ref<RendererContext> ctx, Swapchain& swapchain);

    VkSwapchainKHR swapchainHandle{};
    VkSurfaceKHR surface{};
    VkSurfaceFormatKHR surfaceFormat{};
    VkPresentModeKHR presentMode{};
    VkExtent2D extent{};
    GLFWwindow* windowHandle{};
    u32 imageCount{};
    ImageView imageViews[FRAME_COUNT]{};
    VkFramebuffer framebuffers[FRAME_COUNT]{};

private:
    static VkSurfaceFormatKHR get_surface_format(const std::vector<VkSurfaceFormatKHR>& formats);
    static VkPresentModeKHR get_present_mode(const std::vector<VkPresentModeKHR>& present_modes);
    static VkExtent2D get_surface_extent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
};

}  // namespace toki
