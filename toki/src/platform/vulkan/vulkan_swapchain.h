#pragma once

#include "core/core.h"
#include "core/window.h"
#include "vector"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_framebuffer.h"
#include "vulkan/vulkan_types.h"

namespace Toki {

class VulkanSwapchain {
public:
    VulkanSwapchain() = delete;
    VulkanSwapchain(const VulkanContext* context, const SwapchainConfig& swapchainConfig, const WindowConfig& windowConfig);
    VulkanSwapchain(const VulkanContext* context, const SwapchainConfig& swapchainConfig, Ref<Window> window);
    ~VulkanSwapchain();

    void init();
    void destroy(bool destroyHandle = true);
    void recreate();

    Ref<Window> getWindow() const { return window; }

    VkSwapchainKHR getSwapchain() const { return swapchain; }

    VkSurfaceKHR getSurface() const { return surface; }

    uint32_t getImageIndex() const { return currentImageIndex; }

    const VkExtent2D& getExtent() const { return extent; }

    const std::vector<VkImageView>& getImageViews() const { return imageViews; }

    uint32_t acquireNextImage(FrameData& frameData);

    Ref<VulkanFramebuffer> createFramebuffer(FramebufferConfig framebufferConfig, uint32_t imageIndex);

    static const uint32_t TIMEOUT;

private:
    const VulkanContext* context;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    Ref<Window> window;

    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<Ref<VulkanFramebuffer>> createdFramebuffers[MAX_FRAMES];

    uint32_t currentImageIndex = 0;

    void createSurface();
    void createSwapchain();

    void findSurfaceFormat();
    void findPresentMode();
    void findExtent();
};

}  // namespace Toki
