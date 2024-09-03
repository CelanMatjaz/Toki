#include "swapchain.h"

#include <algorithm>

#include "utils/device_utils.h"
#include "vulkan/vulkan_core.h"

namespace Toki {

TkError create_swapchain(VulkanState* state, RendererWindow* renderer_window) {
    VkSurfaceCapabilitiesKHR surface_capabilities = query_surface_capabilities(&state->physical_device_data, renderer_window->surface);
    uint32_t image_count = std::clamp(MAX_FRAMES, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);
    renderer_window->extent = surface_capabilities.currentExtent;

    VkSurfaceFormatKHR surface_format = select_surface_format(&state->physical_device_data);
    VkPresentModeKHR present_mode = select_present_mode(&state->physical_device_data);

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = renderer_window->surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    swapchain_create_info.imageExtent = surface_capabilities.currentExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = renderer_window->old_swapchain;

    uint32_t queue_family_indices[] = {
        state->physical_device_data.present_queue_family_index,
        state->physical_device_data.graphics_queue_family_index,
    };
    uint32_t queue_family_index_count = sizeof(queue_family_indices) / sizeof(uint32_t);

    if (queue_family_indices[0] != queue_family_indices[1]) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = queue_family_index_count;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkResult result = vkCreateSwapchainKHR(state->device, &swapchain_create_info, state->allocation_callbacks, &renderer_window->swapchain);
    ASSERT_VK_RESULT(result, Error::RENDERER_CREATE_SWAPCHAIN_ERROR);

    if (renderer_window->old_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(state->device, renderer_window->old_swapchain, state->allocation_callbacks);
    }

    renderer_window->old_swapchain = renderer_window->swapchain;

    return TkError{};
}

}  // namespace Toki
