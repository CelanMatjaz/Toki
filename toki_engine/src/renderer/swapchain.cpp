#include "swapchain.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <limits>
#include <vector>

#include "macros.h"
#include "platform/renderer_utils.h"
#include "renderer/data/image_view.h"
#include "utils.h"
#include "vulkan/vulkan_core.h"

namespace toki {

static VkSurfaceFormatKHR get_surface_format(const std::vector<VkSurfaceFormatKHR> &formats);
static VkPresentModeKHR get_present_mode(const std::vector<VkPresentModeKHR> &present_modes);
static VkExtent2D get_surface_extent(
    GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities);

Swapchain::Swapchain(RendererContext *ctx, GLFWwindow *window): m_windowHandle(window) {
    VkSurfaceKHR surface{};
    create_surface(ctx, window, &surface);
    m_surface = Scope<VkSurfaceKHR, VK_NULL_HANDLE>(surface, [&](VkSurfaceKHR s) {
        vkDestroySurfaceKHR(ctx->instance, s, ctx->allocationCallbacks);
    });

    u32 format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicalDevice, m_surface, &format_count, nullptr);
    TK_ASSERT(format_count > 0, "No surface formats found on physical device");
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        ctx->physicalDevice, m_surface, &format_count, surface_formats.data());
    m_surfaceFormat = get_surface_format(surface_formats);

    u32 present_mode_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        ctx->physicalDevice, m_surface, &present_mode_count, nullptr);
    TK_ASSERT(present_mode_count > 0, "No present modes found on physical device");
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        ctx->physicalDevice, m_surface, &present_mode_count, present_modes.data());
    m_presentMode = get_present_mode(present_modes);

    create(ctx);
}

void Swapchain::recreate(RendererContext *ctx) {
    destroy(ctx);
    create(ctx);
}

void Swapchain::create(RendererContext *ctx) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physicalDevice, m_surface, &capabilities);

    m_extent = get_surface_extent(m_windowHandle, capabilities);
    TK_ASSERT(m_extent.width > 0 && m_extent.height > 0, "Surface extent is not of valid size");

    TK_LOG_INFO("width {}, height {}", m_extent.width, m_extent.height);

    u32 image_count = std::clamp(
        static_cast<u32>(FRAME_COUNT), capabilities.minImageCount, capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = m_surfaceFormat.format;
    swapchain_create_info.imageColorSpace = m_surfaceFormat.colorSpace;
    swapchain_create_info.imageExtent = m_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.preTransform = capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = m_presentMode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    const auto found_queue_family_indices = find_queue_families(ctx->physicalDevice, m_surface);
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
        vkCreateSwapchainKHR(
            ctx->device, &swapchain_create_info, ctx->allocationCallbacks, &m_swapchain),
        "Could not create swapchain");

    {
        u32 image_count{};
        vkGetSwapchainImagesKHR(ctx->device, m_swapchain, &image_count, nullptr);
        TK_ASSERT(image_count > 0, "No images found for swapchain");
        std::vector<VkImage> images(image_count);
        vkGetSwapchainImagesKHR(ctx->device, m_swapchain, &image_count, images.data());

        for (i32 i = 0; i < image_count; i++) {
            ImageViewConfig config{
                .image = images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_surfaceFormat.format,
            };
            m_imageViews[i] = create_image_view(ctx, config);
        }
    }
}

void Swapchain::destroy(RendererContext *ctx) {
    for (VkImageView image_view : m_imageViews) {
        vkDestroyImageView(ctx->device, image_view, ctx->allocationCallbacks);
    }

    vkDestroySwapchainKHR(ctx->device, m_swapchain, ctx->allocationCallbacks);
}

static VkSurfaceFormatKHR get_surface_format(const std::vector<VkSurfaceFormatKHR> &formats) {
    for (const auto &f : formats) {
        const auto &[format, color_space] = f;
        if (format == VK_FORMAT_B8G8R8A8_SRGB && color_space == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return f;
        }
    }

    return formats[0];
}

static VkPresentModeKHR get_present_mode(const std::vector<VkPresentModeKHR> &present_modes) {
    for (const auto &present_mode : present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D get_surface_extent(
    GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D extent{ static_cast<u32>(width), static_cast<u32>(height) };

    extent.width = std::clamp(
        extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(
        extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
}

}  // namespace toki
