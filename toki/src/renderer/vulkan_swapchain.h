#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "renderer/vulkan_types.h"
#include "toki/core/core.h"
#include "toki/core/window.h"

namespace Toki {

class VulkanSwapchain {
public:
    VulkanSwapchain() = delete;
    VulkanSwapchain(Ref<VulkanContext> context, const SwapchainConfig& swapchainConfig, const WindowConfig& windowConfig);
    VulkanSwapchain(Ref<VulkanContext> context, const SwapchainConfig& swapchainConfig, Ref<Window> window);
    ~VulkanSwapchain();

    void init();
    void destroy(bool destroyHandle = true);
    void recreate();

    uint32_t acquireNextImage(FrameData& frameData);

    inline static const uint32_t TIMEOUT = UINT32_MAX;

private:
    Ref<VulkanContext> m_context;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    Ref<Window> m_window;

    inline static VkSurfaceFormatKHR s_surfaceFormat{};
    VkPresentModeKHR m_presentMode;
    VkExtent2D m_extent;

    uint32_t m_currentImageIndex = 0;

    void createSwapchain();

    void findSurfaceFormat();
    void findPresentMode();
    void findExtent();
};

}  // namespace Toki
