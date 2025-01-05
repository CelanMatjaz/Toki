#include "swapchain.h"

#include <vulkan/vulkan.h>

#include <algorithm>
#include <limits>
#include <vector>

#include "macros.h"
#include "platform/renderer_utils.h"
#include "utils.h"

namespace toki {

Swapchain Swapchain::create(Ref<RendererContext> ctx, Ref<Window> window) {
    Swapchain swapchain{};

    swapchain.windowHandle = reinterpret_cast<GLFWwindow*>(window->get_handle());
    swapchain.surface = create_surface(ctx, swapchain.windowHandle);

    u32 format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicalDevice, swapchain.surface, &format_count, nullptr);
    TK_ASSERT(format_count > 0, "No surface formats found on physical device");
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicalDevice, swapchain.surface, &format_count, surface_formats.data());
    swapchain.surfaceFormat = get_surface_format(surface_formats);

    u32 present_mode_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physicalDevice, swapchain.surface, &present_mode_count, nullptr);
    TK_ASSERT(present_mode_count > 0, "No present modes found on physical device");
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physicalDevice, swapchain.surface, &present_mode_count, present_modes.data());
    swapchain.presentMode = get_present_mode(present_modes);

    Swapchain::recreate(ctx, swapchain);

    return swapchain;
}

void Swapchain::cleanup(Ref<RendererContext> ctx, Swapchain& swapchain) {
    for (ImageView& image_view : swapchain.imageViews) {
        ImageView::cleanup(ctx, image_view);
    }

    vkDestroySwapchainKHR(ctx->device, swapchain.swapchainHandle, ctx->allocationCallbacks);
    vkDestroySurfaceKHR(ctx->instance, swapchain.surface, ctx->allocationCallbacks);
}

void Swapchain::recreate(Ref<RendererContext> ctx, Swapchain& swapchain) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physicalDevice, swapchain.surface, &capabilities);

    swapchain.extent = get_surface_extent(swapchain.windowHandle, capabilities);
    TK_ASSERT(swapchain.extent.width > 0 && swapchain.extent.height > 0, "Surface extent is not of valid size");

    u32 image_count = std::clamp(static_cast<u32>(FRAME_COUNT), capabilities.minImageCount, capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = swapchain.surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = swapchain.surfaceFormat.format;
    swapchain_create_info.imageColorSpace = swapchain.surfaceFormat.colorSpace;
    swapchain_create_info.imageExtent = swapchain.extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = swapchain.presentMode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = swapchain.swapchainHandle;

    const auto found_queue_family_indices = find_queue_families(ctx->physicalDevice, swapchain.surface);
    u32 queue_family_indices[] = { static_cast<u32>(found_queue_family_indices.graphics),
                                   static_cast<u32>(found_queue_family_indices.present) };

    if (found_queue_family_indices.graphics != found_queue_family_indices.present) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }

    TK_ASSERT_VK_RESULT(
        vkCreateSwapchainKHR(ctx->device, &swapchain_create_info, ctx->allocationCallbacks, &swapchain.swapchainHandle),
        "Could not create swapchain");

    if (swapchain_create_info.oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(ctx->device, swapchain_create_info.oldSwapchain, ctx->allocationCallbacks);
    }

    {
        vkGetSwapchainImagesKHR(ctx->device, swapchain.swapchainHandle, &swapchain.imageCount, nullptr);
        TK_ASSERT(swapchain.imageCount > 0, "No images found for swapchain");
        std::vector<VkImage> images(swapchain.imageCount);
        vkGetSwapchainImagesKHR(ctx->device, swapchain.swapchainHandle, &swapchain.imageCount, images.data());

        for (u32 i = 0; i < swapchain.imageCount; i++) {
            ImageView::Config image_view_config{};
            image_view_config.image = images[i];
            image_view_config.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_config.format = swapchain.surfaceFormat.format;
            swapchain.imageViews[i] = ImageView::create(ctx, image_view_config);
        }
    }
}

VkSurfaceFormatKHR Swapchain::get_surface_format(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (const auto& f : formats) {
        const auto& [format, color_space] = f;
        if (format == VK_FORMAT_B8G8R8A8_SRGB && color_space == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return f;
        }
    }

    return formats[0];
}

VkPresentModeKHR Swapchain::get_present_mode(const std::vector<VkPresentModeKHR>& present_modes) {
    for (const auto& present_mode : present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::get_surface_extent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D extent{ static_cast<u32>(width), static_cast<u32>(height) };
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
}

}  // namespace toki
