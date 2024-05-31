#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "renderer/texture.h"
#include "toki/renderer/renderer_types.h"
#include "toki/resources/configs.h"

namespace Toki {

struct Texture;

struct Swapchain {
    Swapchain() = default;
    Swapchain(VkSurfaceKHR surface);
    ~Swapchain();

    void create();
    void destroy();
    void recreate();

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkSwapchainKHR m_oldSwapchain = VK_NULL_HANDLE;
    VkExtent2D m_extent{};
    VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t m_imageIndex;
    bool m_isRendering = false;

    std::vector<Ref<WrappedImage>> m_wrappedImages;
};

}  // namespace Toki
