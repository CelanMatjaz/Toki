#include "swapchain.h"

#include "renderer/texture.h"
#include "renderer/vulkan_types.h"
#include "toki/renderer/renderer_types.h"

namespace Toki {

Swapchain::Swapchain(VkSurfaceKHR surface) : m_surface(surface) {
    create();
}

Swapchain::~Swapchain() {
    if (m_surface != VK_NULL_HANDLE) {
        destroy();
        vkDestroySurfaceKHR(context.instance, m_surface, context.allocationCallbacks);
    }
}

static VkExtent2D findExtent(VkSurfaceKHR surface);
// static VkSurfaceFormatKHR findSurfaceFormat();
// static VkPresentModeKHR findPresentMode();

void Swapchain::create() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDeviceData.physicalDevice, m_surface, &surfaceCapabilities);

    uint32_t imageCount = std::clamp<uint32_t>(MAX_FRAMES, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
    m_extent = findExtent(m_surface);

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = m_surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = context.physicalDeviceData.presentableSurfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = context.physicalDeviceData.presentableSurfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = m_extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = m_presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = m_oldSwapchain;

    uint32_t queueFamilyIndecies[] = { context.physicalDeviceData.graphicsFamilyIndex.value(),
                                       context.physicalDeviceData.presentFamilyIndex.value() };

    if (queueFamilyIndecies[0] != queueFamilyIndecies[1]) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndecies;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    TK_ASSERT_VK_RESULT(
        vkCreateSwapchainKHR(context.device, &swapchainCreateInfo, context.allocationCallbacks, &m_swapchain), "Could not create swapchain");

    if (m_oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(context.device, m_oldSwapchain, context.allocationCallbacks);
    }

    m_oldSwapchain = m_swapchain;

    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(context.device, m_swapchain, &swapchainImageCount, nullptr);
    std::vector<VkImage> images(swapchainImageCount);
    vkGetSwapchainImagesKHR(context.device, m_swapchain, &swapchainImageCount, images.data());

    m_wrappedImages.resize(images.size());
    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        m_wrappedImages[i] = createRef<WrappedImage>(images[i], context.physicalDeviceData.presentableSurfaceFormat.format);
    }
}

void Swapchain::destroy() {
    m_wrappedImages.clear();
    vkDestroySwapchainKHR(context.device, m_swapchain, context.allocationCallbacks);
    m_oldSwapchain = VK_NULL_HANDLE;
}

void Swapchain::recreate() {
    destroy();
    m_imageIndex = 0;
    create();
}

static VkExtent2D findExtent(VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDeviceData.physicalDevice, surface, &surfaceCapabilities);

    if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
        return surfaceCapabilities.currentExtent;
    }

    std::unreachable();
}

// static VkSurfaceFormatKHR findSurfaceFormat() {
//     for (const auto& format : context.physicalDeviceData.surfaceFormats) {
//         if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
//             return format;
//         }
//     }
//
//     return context.physicalDeviceData.surfaceFormats[0];
// }
//
// static VkPresentModeKHR findPresentMode() {
//     for (const auto& presentMode : context.physicalDeviceData.presentModes) {
//         if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
//             return presentMode;
//         }
//     }
//
//     return VK_PRESENT_MODE_FIFO_KHR;
// }

}  // namespace Toki
