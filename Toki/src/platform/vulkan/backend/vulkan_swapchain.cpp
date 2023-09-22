#include "tkpch.h"
#include "toki/core/engine.h"
#include "toki/core/assert.h"
#include "vulkan_swapchain.h"
#include "vulkan_utils.h"
#include <platform/vulkan/vulkan_renderer.h>

namespace Toki {

    VulkanSwapchain::VulkanSwapchain(VkPresentModeKHR presentMode) {
        recreate(presentMode);
    }

    VulkanSwapchain::~VulkanSwapchain() {
        destroy();
    }

    void VulkanSwapchain::recreate(VkPresentModeKHR presentMode) {
        VkDevice device = VulkanRenderer::device();
        VkPhysicalDevice physicalDevice = Toki::VulkanRenderer::physicalDevice();
        VkSurfaceKHR surface = Toki::VulkanRenderer::surface();

        const auto [graphicsQueueIndex, presentQueueIndex] = Toki::VulkanRenderer::queueFamilyIndexes();
        const auto windowWidth = Engine::getWindow()->getWidth();
        const auto windowHeight = Engine::getWindow()->getHeight();

        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, formats.data());

        auto format = VulkanUtils::findSurfaceFormat();

        extent = VulkanUtils::getExtent();
        imageFormat = format.format;

        VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        VkSurfaceTransformFlagBitsKHR preTransform = VulkanUtils::getPreTransform();
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VulkanUtils::getCompositeAlpha();
        uint32_t imageCount = VulkanUtils::getImageCount(VulkanContext::MAX_FRAMES);

        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = imageFormat;
        swapchainCreateInfo.imageColorSpace = format.colorSpace;
        swapchainCreateInfo.imageExtent = extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.compositeAlpha = compositeAlpha;
        swapchainCreateInfo.presentMode = swapchainPresentMode;
        swapchainCreateInfo.clipped = true;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.preTransform = preTransform;

        uint32_t queueFamilyIndecies[2] = { graphicsQueueIndex, presentQueueIndex };
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndecies;

        if (queueFamilyIndecies[0] != queueFamilyIndecies[1]) {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount = 2;
        }
        else {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        TK_ASSERT_VK_RESULT(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain), "Could not create swapchain");

        std::vector<VkImage> swapchainImages;
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
        swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

        images.resize(imageCount);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.format = imageFormat;
        imageViewCreateInfo.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        };
        imageViewCreateInfo.subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
        };
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.flags = 0;

        for (uint32_t i = 0; i < imageCount; ++i) {
            imageViewCreateInfo.image = swapchainImages[i];
            VkImageView imageView;
            vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView);
            images[i] = createRef<VulkanImage>(swapchainImages[i], imageView, imageFormat);
        }

        VulkanImageConfig imageConfig{};
        imageConfig.format = VulkanUtils::findDepthFormat();
        imageConfig.tiling = VulkanUtils::findTiling(imageConfig.format);
        imageConfig.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageConfig.extent = { extent.width, extent.height, 1 };
        depthBuffer = createRef<VulkanImage>(imageConfig);
    }

    void VulkanSwapchain::destroy() {
        VkDevice device = Toki::VulkanRenderer::device();

        vkDestroySwapchainKHR(device, swapchain, nullptr);

        depthBuffer.reset();
    }

}