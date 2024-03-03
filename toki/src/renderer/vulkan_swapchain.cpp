#include "vulkan_swapchain.h"

#include <algorithm>
#include <format>

#include "platform.h"
#include "renderer/vulkan_utils.h"
#include "toki/core/assert.h"
#include "toki/core/core.h"
#include "toki/core/window.h"

#ifdef TK_WINDOW_SYSTEM_GLFW
#include <GLFW/glfw3.h>
#endif

namespace Toki {

VulkanSwapchain::VulkanSwapchain(Ref<VulkanContext> context, const SwapchainConfig& swapchainConfig, const WindowConfig& windowConfig)
    : VulkanSwapchain(context, swapchainConfig, Window::create(windowConfig)) {}

VulkanSwapchain::VulkanSwapchain(Ref<VulkanContext> context, const SwapchainConfig& swapchainConfig, Ref<Window> window)
    : m_context(context),
      m_window(window),
      m_surface(VulkanUtils::createSurface(context, window)) {
    findSurfaceFormat();
    findPresentMode();
    findExtent();
}

VulkanSwapchain::~VulkanSwapchain() {
    destroy();
    vkDestroySurfaceKHR(m_context->instance, m_surface, m_context->allocationCallbacks);
}

void VulkanSwapchain::init() {
    createSwapchain();
}

void VulkanSwapchain::destroy(bool destroyHandle) {
    vkDeviceWaitIdle(m_context->device);
}

void VulkanSwapchain::recreate() {
    vkDeviceWaitIdle(m_context->device);
}

void VulkanSwapchain::createSwapchain() {
    m_currentImageIndex = 0;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->physicalDevice, m_surface, &surfaceCapabilities);

    uint32_t imageCount = std::clamp<uint32_t>(MAX_FRAMES, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

    static VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = m_surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = s_surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = s_surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = m_extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = m_presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = oldSwapchain;

    uint32_t queueFamilyIndecies[] = { m_context->graphicsFamilyIndex, m_context->presentFamilyIndex };

    if (m_context->graphicsFamilyIndex != m_context->presentFamilyIndex) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndecies;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    TK_ASSERT_VK_RESULT(
        vkCreateSwapchainKHR(m_context->device, &swapchainCreateInfo, m_context->allocationCallbacks, &m_swapchain), "Could not create swapchain"
    );

    if (oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_context->device, oldSwapchain, m_context->allocationCallbacks);
    }

    oldSwapchain = m_swapchain;

    // uint32_t swapchainImageCount = 0;
    // vkGetSwapchainImagesKHR(m_context->device, m_swapchain, &swapchainImageCount, nullptr);
    // m_images.resize(swapchainImageCount);
    // vkGetSwapchainImagesKHR(m_context->device, m_swapchain, &swapchainImageCount, m_images.data());

    // wrappedImages.resize(swapchainImageCount);
    // for (uint32_t i = 0; i < swapchainImageCount; ++i) {
    //     wrappedImages[i] = createRef<InternalVulkanTexture>(m_images[i], swapchainCreateInfo.imageFormat);
    // }
}

uint32_t VulkanSwapchain::acquireNextImage(FrameData& frameData) {
    VkResult result = vkAcquireNextImageKHR(m_context->device, m_swapchain, TIMEOUT, frameData.presentSemaphore, nullptr, &m_currentImageIndex);

    VkResult waitFencesResult = vkWaitForFences(m_context->device, 1, &frameData.renderFence, VK_TRUE, TIMEOUT);
    TK_ASSERT(waitFencesResult == VK_SUCCESS || waitFencesResult == VK_TIMEOUT, "Failed waiting for fences");

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return MAX_FRAMES;
    } else {
        TK_ASSERT(
            result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, std::format("Failed to acquire swapchain image index, result: {}", (uint32_t) result)
        );
    }

    return m_currentImageIndex;
}

void VulkanSwapchain::findSurfaceFormat() {
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->physicalDevice, m_surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    if (formatCount > 0) {
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->physicalDevice, m_surface, &formatCount, surfaceFormats.data());
    }

    for (const auto& format : surfaceFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            s_surfaceFormat = format;
            return;
        }
    }

    s_surfaceFormat = surfaceFormats[0];
}

void VulkanSwapchain::findPresentMode() {
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_context->physicalDevice, m_surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    if (presentModeCount) {
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_context->physicalDevice, m_surface, &presentModeCount, nullptr);
    }

    for (const auto& presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            m_presentMode = presentMode;
        }
    }

    m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void VulkanSwapchain::findExtent() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->physicalDevice, m_surface, &surfaceCapabilities);

    if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
        m_extent = surfaceCapabilities.currentExtent;
        return;
    }

    auto [width, height] = m_window->getDimensions();

    m_extent = {
        std::clamp<uint32_t>(width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height),
    };
}

}  // namespace Toki
