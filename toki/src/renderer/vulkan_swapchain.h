#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

#include "renderer/vulkan_image.h"
#include "renderer/vulkan_types.h"
#include "toki/core/core.h"
#include "toki/core/window.h"

namespace Toki {

class VulkanSwapchain {
public:
    static Ref<VulkanSwapchain> create(Ref<VulkanContext> context, const SwapchainConfig& swapchainConfig, Ref<Window> window);

    VulkanSwapchain() = delete;
    VulkanSwapchain(Ref<VulkanContext> context, const SwapchainConfig& swapchainConfig, Ref<Window> window);
    ~VulkanSwapchain();

    void init();
    void destroy();

    void recreate();

    void transitionLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout);

    std::optional<uint32_t> acquireNextImage(FrameData& frameData);
    VkSwapchainKHR getSwapchainHandle() const;
    uint32_t getCurrentImageIndex() const;
    VkImageView getCurrentImageView() const;
    Ref<Window> getWindow() const;
    VkExtent2D getExtent() const;

    inline static const uint32_t TIMEOUT = UINT32_MAX;

private:
    Ref<VulkanContext> m_context;
    Ref<Window> m_window;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    bool m_useVSync : 1 = false;

    inline static VkSurfaceFormatKHR s_surfaceFormat{};
    VkPresentModeKHR m_presentMode;
    VkExtent2D m_extent;

    std::vector<Ref<VulkanImage>> m_wrappedImages;

    uint32_t m_currentImageIndex = 0;

    void createSwapchainHandle();

    void findSurfaceFormat();
    void findPresentMode();
    void findExtent();
};

}  // namespace Toki
