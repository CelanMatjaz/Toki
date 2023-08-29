#pragma once

#include "vulkan/vulkan.h"
#include "core/core.h"
#include "vulkan_image.h"
#include "vector"

namespace Toki {

    class VulkanSwapchain {
    public:
        VulkanSwapchain(VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR);
        ~VulkanSwapchain();

        void recreate(VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR);
        void destroy();

        VkSwapchainKHR getHandle() { return swapchain; }
        VkExtent2D getExtent() { return extent; }
        VkFormat getFormat() { return imageFormat; }

        std::vector<Ref<VulkanImage>> images;
        Ref<VulkanImage> depthBuffer;

    private:
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkExtent2D extent;
        VkFormat imageFormat;
    };

}