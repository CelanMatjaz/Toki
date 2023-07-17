#include "vulkan_swapchain.h"

#include "toki/core/application.h"
#include "vulkan_device.h"
#include "infos.h"

namespace Toki {

    VulkanSwapchain::VulkanSwapchain(VkPresentModeKHR presentMode) {
        create(presentMode);
    }

    VulkanSwapchain::~VulkanSwapchain() {
        cleanup();
    }

    void  VulkanSwapchain::recreate(VkPresentModeKHR presentMode) {
        cleanup();
        create(presentMode);
    }

    void VulkanSwapchain::create(VkPresentModeKHR presentMode) {
        VulkanRenderer* renderer = Application::getVulkanRenderer();
        const auto [graphicsQueueIndex, presentQueueIndex] = renderer->getQueueFamilyIndexes();
        const auto [windowWidth, windowHeight] = Application::getWindow()->getWindowDimensions();

        VkSurfaceKHR surface = renderer->getSurface();
        VkPhysicalDevice physicalDevice = renderer->getPhysicalDevice();
        VkDevice device = renderer->getDevice();

        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, formats.data());

        imageFormat = VulkanDevice::findSurfaceFormat(formats).format;
        extent = VulkanDevice::getExtent();

        VkPresentModeKHR swapchainPresentMode = presentMode;
        VkSurfaceTransformFlagBitsKHR preTransform = VulkanDevice::getPreTransform();
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VulkanDevice::getCompositeAlpha();
        uint32_t imageCount = VulkanDevice::getImageCount(VulkanRenderer::MAX_FRAMES);

        VkSwapchainCreateInfoKHR swapchainCreateInfo = Infos::Renderer::swapchainCreateInfo();
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = imageFormat;
        swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainCreateInfo.imageExtent = extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.compositeAlpha = compositeAlpha;
        swapchainCreateInfo.presentMode = swapchainPresentMode;
        swapchainCreateInfo.clipped = true;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.preTransform = preTransform;
        swapchainCreateInfo.queueFamilyIndexCount = 0;

        uint32_t queueFamilyIndecies[2] = { graphicsQueueIndex, presentQueueIndex };

        if (queueFamilyIndecies[0] != queueFamilyIndecies[1]) {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount = 2;
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndecies;
        }

        TK_ASSERT(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) == VK_SUCCESS);

        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

        imageViews.resize(imageCount);
        VkImageViewCreateInfo imageViewCreateInfo = Infos::Container::imageViewCreateInfo(nullptr, imageFormat);

        for (uint32_t i = 0; i < imageCount; ++i) {
            imageViewCreateInfo.image = images[i];
            TK_ASSERT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]) == VK_SUCCESS);
        }

        createDepthBuffer();
    }

    void VulkanSwapchain::cleanup() {
        VkDevice device = Application::getVulkanRenderer()->getDevice();

        vkDeviceWaitIdle(device);

        for (auto& imageView : imageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    void VulkanSwapchain::createDepthBuffer() {
        VulkanRenderer* renderer = Application::getVulkanRenderer();
        const auto [graphicsQueueIndex, presentQueueIndex] = renderer->getQueueFamilyIndexes();
        const auto [windowWidth, windowHeight] = Application::getWindow()->getWindowDimensions();

        VkSurfaceKHR surface = renderer->getSurface();
        VkDevice device = renderer->getDevice();
        VkPhysicalDevice physicalDevice = renderer->getPhysicalDevice();

        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

        depthFormat = VulkanDevice::findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );

        VkExtent2D extent = VulkanDevice::getExtent();
        VkImageTiling tiling = VulkanDevice::findTiling(depthFormat);

        VkImageCreateInfo imageCreateInfo = Infos::Container::imageCreateInfo(depthFormat, tiling, { extent.width, extent.height, 1 }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateImage(device, &imageCreateInfo, nullptr, &depthBuffer.image);

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(device, depthBuffer.image, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo = Infos::Container::memoryAllocateInfo(memoryRequirements.size, VulkanDevice::findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        TK_ASSERT(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &depthBuffer.memory) == VK_SUCCESS);
        TK_ASSERT(vkBindImageMemory(device, depthBuffer.image, depthBuffer.memory, 0) == VK_SUCCESS);

        VkImageViewCreateInfo imageViewCreateInfo = Infos::Container::imageViewCreateInfo(depthBuffer.image, depthFormat);
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        TK_ASSERT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &depthBuffer.imageView) == VK_SUCCESS);
    }

}