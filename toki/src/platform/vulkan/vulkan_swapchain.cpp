#include "vulkan_swapchain.h"

#include "GLFW/glfw3.h"
#include "algorithm"
#include "core/assert.h"
#include "core/core.h"
#include "core/window.h"
#include "glfw/glfw_window.h"
#include "mutex"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_framebuffer.h"
#include "vulkan/vulkan_image.h"
#include "vulkan/vulkan_renderer.h"
#include "vulkan/vulkan_types.h"
#include "windows/windows_window.h"

namespace Toki {

const uint32_t VulkanSwapchain::TIMEOUT = UINT64_MAX;

VulkanSwapchain::VulkanSwapchain(const VulkanContext* context, const SwapchainConfig& swapchainConfig, const WindowConfig& windowConfig)
    :
#if defined(WINDOW_SYSTEM_WINDOWS)
      VulkanSwapchain(context, swapchainConfig, createRef<WindowsWindow>(windowConfig))
#elif defined(WINDOW_SYSTEM_GLFW)
      VulkanSwapchain(context, swapchainConfig, createRef<GlfwWindow>(windowConfig))
#endif
{
}

VulkanSwapchain::VulkanSwapchain(const VulkanContext* context, const SwapchainConfig& swapchainConfig, Ref<Window> window)
    : context(context),
      window(window) {
    createSurface();

    findSurfaceFormat();
    findPresentMode();
    findExtent();
}

VulkanSwapchain::~VulkanSwapchain() {
    destroy();
    vkDestroySurfaceKHR(context->instance, surface, context->allocationCallbacks);
}

void VulkanSwapchain::init() {
    createSwapchain();
}

void VulkanSwapchain::destroy(bool destroyHandle) {
    vkDeviceWaitIdle(context->device);

    for (const auto& imageView : imageViews) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(context->device, imageView, context->allocationCallbacks);
        }
    }

    if (destroyHandle && swapchain != VK_NULL_HANDLE) {
        for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
            createdFramebuffers[i].clear();
        }

        vkDestroySwapchainKHR(context->device, swapchain, context->allocationCallbacks);
        swapchain = VK_NULL_HANDLE;
    }
}

void VulkanSwapchain::recreate() {
    vkDeviceWaitIdle(context->device);

    destroy(false);

    findExtent();

    createSwapchain();

    for (uint32_t imageIndex = 0; imageIndex < MAX_FRAMES; ++imageIndex) {
        for (uint32_t fbIndex = 0; fbIndex < createdFramebuffers[imageIndex].size(); ++fbIndex) {
            createdFramebuffers[imageIndex][fbIndex]->recreate(extent.width, extent.height, 1, imageViews[imageIndex]);
        }
    }
}

void VulkanSwapchain::createSurface() {
#if defined(WINDOW_SYSTEM_WINDOWS)
    VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo{};
    win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32SurfaceCreateInfo.hinstance = GetModuleHandle(0);
    win32SurfaceCreateInfo.hwnd = (HWND) window->getHandle();
    TK_ASSERT_VK_RESULT(
        vkCreateWin32SurfaceKHR(context->instance, &win32SurfaceCreateInfo, context->allocationCallbacks, &surface), "Could not create surface"
    );

#elif defined(WINDOW_SYSTEM_GLFW)
    TK_ASSERT_VK_RESULT(
        glfwCreateWindowSurface(context->instance, (GLFWwindow*) window->getHandle(), context->allocationCallbacks, &surface),
        "Could not create surface"
    );
#endif
}

void VulkanSwapchain::createSwapchain() {
    currentImageIndex = 0;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, surface, &surfaceCapabilities);

    uint32_t imageCount = std::clamp<uint32_t>(MAX_FRAMES, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

    static VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = oldSwapchain;

    uint32_t queueFamilyIndecies[] = { context->graphicsFamilyIndex, context->presentFamilyIndex };

    if (context->graphicsFamilyIndex != context->presentFamilyIndex) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndecies;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    TK_ASSERT_VK_RESULT(
        vkCreateSwapchainKHR(context->device, &swapchainCreateInfo, context->allocationCallbacks, &swapchain), "Could not create swapchain"
    );

    if (oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(context->device, oldSwapchain, context->allocationCallbacks);
    }

    oldSwapchain = swapchain;

    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(context->device, swapchain, &swapchainImageCount, nullptr);
    images.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(context->device, swapchain, &swapchainImageCount, images.data());

    imageViews.resize(swapchainImageCount);
    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        imageViews[i] = VulkanImage::createImageView(context, { images[i], surfaceFormat.format, VK_IMAGE_VIEW_TYPE_2D });
    }
}

uint32_t VulkanSwapchain::acquireNextImage(FrameData& frameData) {
    VkResult result = vkAcquireNextImageKHR(context->device, swapchain, TIMEOUT, frameData.presentSemaphore, nullptr, &currentImageIndex);

    VkResult waitFencesResult = vkWaitForFences(context->device, 1, &frameData.renderFence, VK_TRUE, TIMEOUT);
    TK_ASSERT(waitFencesResult == VK_SUCCESS || waitFencesResult == VK_TIMEOUT, "Failed waiting for fences");

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // this->recreate();
        return MAX_FRAMES;
    } else {
        TK_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire swapchain image index");
    }

    return currentImageIndex;
}

Ref<VulkanFramebuffer> VulkanSwapchain::createFramebuffer(FramebufferConfig framebufferConfig, uint32_t imageIndex) {
    TK_ASSERT(imageIndex < MAX_FRAMES, "Cannot create framebuffer for index more than MAX_FRAMES - 1");
    framebufferConfig.width = extent.width;
    framebufferConfig.height = extent.height;
    framebufferConfig.attachments.emplace(framebufferConfig.attachments.begin(), imageViews[imageIndex]);
    createdFramebuffers[imageIndex].emplace_back(std::move(VulkanFramebuffer::create(context, framebufferConfig)));
    return createdFramebuffers[imageIndex].back();
}

void VulkanSwapchain::findSurfaceFormat() {
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    if (formatCount > 0) {
        vkGetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, surface, &formatCount, surfaceFormats.data());
    }

    for (const auto& format : surfaceFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            this->surfaceFormat = format;
            return;
        }
    }

    this->surfaceFormat = surfaceFormats[0];
}

void VulkanSwapchain::findPresentMode() {
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->physicalDevice, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    if (presentModeCount) {
        vkGetPhysicalDeviceSurfacePresentModesKHR(context->physicalDevice, surface, &presentModeCount, nullptr);
    }

    for (const auto& presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            this->presentMode = presentMode;
        }
    }

    this->presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void VulkanSwapchain::findExtent() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, surface, &surfaceCapabilities);

    if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
        this->extent = surfaceCapabilities.currentExtent;
        return;
    }

    auto [width, height] = window->getDimensions();

    this->extent = {
        std::clamp<uint32_t>(width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height),
    };
}

}  // namespace Toki
