#pragma once

#include <GLFW/glfw3.h>

#include "core/core.h"
#include "renderer_state.h"

namespace toki {

class Renderer;

class Swapchain {
    friend Renderer;

public:
    Swapchain() = delete;
    Swapchain(RendererContext* ctx, GLFWwindow* window);
    ~Swapchain() = default;

    DELETE_COPY(Swapchain)
    DELETE_MOVE(Swapchain)

    void recreate(RendererContext* ctx);
    void create(RendererContext* ctx);
    void destroy(RendererContext* ctx);

private:
    GLFWwindow* m_windowHandle;
    Scope<VkSurfaceKHR, VK_NULL_HANDLE> m_surface{};

    VkSwapchainKHR m_swapchain{};
    VkSurfaceFormatKHR m_surfaceFormat{};
    VkPresentModeKHR m_presentMode{};
    VkExtent2D m_extent{};

    VkImageView m_imageViews[FRAME_COUNT]{};
};

}  // namespace toki
