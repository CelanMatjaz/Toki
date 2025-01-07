#include "vulkan_swapchain.h"

#include "core/logging.h"
#include "renderer/vulkan/platform/vulkan_platform.h"
#include "renderer/vulkan/vulkan_framebuffer.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"

namespace toki {

static VkSurfaceFormatKHR get_surface_format(const std::vector<VkSurfaceFormatKHR>& formats);
static VkPresentModeKHR get_present_mode(const std::vector<VkPresentModeKHR>& present_modes);
static VkExtent2D get_surface_extent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

vulkan_swapchain vulkan_swapchain_create(ref<renderer_context> ctx, const create_vulkan_swapchain_config& config) {
    vulkan_swapchain swapchain{};
    swapchain.window = reinterpret_cast<GLFWwindow*>(config.window->get_handle());
    swapchain.surface = create_surface(ctx, swapchain.window);

    u32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, swapchain.surface, &format_count, nullptr);
    TK_ASSERT(format_count > 0, "No surface formats found on physical device");
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, swapchain.surface, &format_count, surface_formats.data());
    swapchain.surface_format = get_surface_format(surface_formats);

    u32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical_device, swapchain.surface, &present_mode_count, nullptr);
    TK_ASSERT(present_mode_count > 0, "No present modes found on physical device");
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical_device, swapchain.surface, &present_mode_count, present_modes.data());
    swapchain.present_mode = get_present_mode(present_modes);

    vulkan_swapchain_recreate(ctx, swapchain);

    {
        if (config.command_pool == VK_NULL_HANDLE) {
            VkCommandPoolCreateInfo command_pool_create_info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            command_pool_create_info.queueFamilyIndex = ctx->queue_family_indices.graphics;
            VK_CHECK(vkCreateCommandPool(ctx->device, &command_pool_create_info, ctx->allocation_callbacks, &swapchain.command_pool), "Could not create dedicated swapchain command pool");
        } else {
            swapchain.command_pool = config.command_pool;
        }

        VkCommandBufferAllocateInfo command_buffer_allocate_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = swapchain.image_count;
        command_buffer_allocate_info.commandPool = swapchain.command_pool;

        VkCommandBuffer swapchain_command_buffers[FRAME_COUNT];
        VK_CHECK(vkAllocateCommandBuffers(ctx->device, &command_buffer_allocate_info, swapchain_command_buffers), "Could not allocate swapchain command buffers")

        for (u32 i = 0; i < swapchain.image_count; i++) {
            swapchain.frames[i].command.handle = swapchain_command_buffers[i];
            swapchain.frames[i].command.state = vulkan_command_buffer_state::READY;
        }
    }

    {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (uint32_t i = 0; i < swapchain.image_count; ++i) {
            VK_CHECK(vkCreateFence(ctx->device, &fence_create_info, ctx->allocation_callbacks, &swapchain.frames[i].render_fence), "Could not create render fence");
            VK_CHECK(vkCreateSemaphore(ctx->device, &semaphore_create_info, ctx->allocation_callbacks, &swapchain.frames[i].render_semaphore), "Could not create render semaphore");
            VK_CHECK(vkCreateSemaphore(ctx->device, &semaphore_create_info, ctx->allocation_callbacks, &swapchain.frames[i].present_semaphore), "Could not create present semaphore");
        }
    }

    return swapchain;
}

void vulkan_swapchain_destroy(ref<renderer_context> ctx, vulkan_swapchain& swapchain) {
    for (uint32_t i = 0; i < FRAME_COUNT; ++i) {
        vkDestroyFence(ctx->device, swapchain.frames[i].render_fence, ctx->allocation_callbacks);
        swapchain.frames[i].render_fence = VK_NULL_HANDLE;
        vkDestroySemaphore(ctx->device, swapchain.frames[i].render_semaphore, ctx->allocation_callbacks);
        swapchain.frames[i].render_semaphore = VK_NULL_HANDLE;
        vkDestroySemaphore(ctx->device, swapchain.frames[i].present_semaphore, ctx->allocation_callbacks);
        swapchain.frames[i].present_semaphore = VK_NULL_HANDLE;
    }

    for (u32 i = 0; i < swapchain.image_count; i++) {
        vkDestroyImageView(ctx->device, swapchain.image_views[i], ctx->allocation_callbacks);
        swapchain.image_views[i] = VK_NULL_HANDLE;
    }

    vkDestroySwapchainKHR(ctx->device, swapchain.handle, ctx->allocation_callbacks);
    swapchain.handle = VK_NULL_HANDLE;
    vkDestroySurfaceKHR(ctx->instance, swapchain.surface, ctx->allocation_callbacks);
    swapchain.surface = VK_NULL_HANDLE;
}

void vulkan_swapchain_recreate(ref<renderer_context> ctx, vulkan_swapchain& swapchain) {
    TK_LOG_INFO("(Re)creating swapchain");

    for (u32 i = 0; i < swapchain.image_count; i++) {
        if (swapchain.image_views[i] == VK_NULL_HANDLE) {
            continue;
        }
        vkDestroyImageView(ctx->device, swapchain.image_views[i], ctx->allocation_callbacks);
        swapchain.image_views[i] = VK_NULL_HANDLE;
    }

    swapchain.current_frame = 0;

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physical_device, swapchain.surface, &capabilities);

    swapchain.extent = get_surface_extent(swapchain.window, capabilities);
    TK_ASSERT(swapchain.extent.width > 0 && swapchain.extent.height > 0, "Surface extent is not of valid size");

    swapchain.image_count = std::clamp(static_cast<u32>(FRAME_COUNT), capabilities.minImageCount, capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = swapchain.surface;
    swapchain_create_info.minImageCount = swapchain.image_count;
    swapchain_create_info.imageFormat = swapchain.surface_format.format;
    swapchain_create_info.imageColorSpace = swapchain.surface_format.colorSpace;
    swapchain_create_info.imageExtent = swapchain.extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = swapchain.present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = swapchain.handle;

    const auto found_queue_family_indices = ctx->queue_family_indices;
    u32 queue_family_indices[] = { static_cast<u32>(found_queue_family_indices.graphics), static_cast<u32>(found_queue_family_indices.present) };

    if (found_queue_family_indices.graphics != found_queue_family_indices.present) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }

    VK_CHECK(vkCreateSwapchainKHR(ctx->device, &swapchain_create_info, ctx->allocation_callbacks, &swapchain.handle), "Could not create swapchain");

    if (swapchain_create_info.oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(ctx->device, swapchain_create_info.oldSwapchain, ctx->allocation_callbacks);
    }

    {
        vkGetSwapchainImagesKHR(ctx->device, swapchain.handle, &swapchain.image_count, swapchain.images);
        TK_ASSERT(swapchain.image_count > 0, "No images found for swapchain");

        VkImageViewCreateInfo image_view_create_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = swapchain.surface_format.format;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.layerCount = 1;

        for (u32 i = 0; i < swapchain.image_count; i++) {
            image_view_create_info.image = swapchain.images[i];
            VK_CHECK(vkCreateImageView(ctx->device, &image_view_create_info, ctx->allocation_callbacks, &swapchain.image_views[i]), "Could not create swapchain image view");
        }
    }

    for (auto& framebuffer : ctx->framebuffers) {
        vulkan_framebuffer_resize(ctx, VkExtent3D{ swapchain.extent.width, swapchain.extent.height, 1 }, framebuffer);
    }
}

b8 vulkan_swapchain_start_recording(ref<renderer_context> ctx, vulkan_swapchain& swapchain) {
    frame& frame = swapchain.frames[swapchain.current_frame];
    VkResult result = vkAcquireNextImageKHR(ctx->device, swapchain.handle, UINT64_MAX, frame.present_semaphore, VK_NULL_HANDLE, &swapchain.current_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vulkan_swapchain_recreate(ctx, swapchain);
        return false;
    } else {
        TK_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Could not acquire swapchain image");
    }

    VkCommandBufferBeginInfo command_buffer_begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(frame.command.handle, &command_buffer_begin_info);
    frame.command.state = vulkan_command_buffer_state::RECORDING_STARTED;

    return true;
}

void vulkan_swapchain_stop_recording(ref<renderer_context> ctx, vulkan_swapchain& swapchain) {
    frame& frame = swapchain.frames[swapchain.current_image_index];
    vkEndCommandBuffer(frame.command.handle);
    frame.command.state = vulkan_command_buffer_state::RECORDING_STOPPED;
}

static VkSurfaceFormatKHR get_surface_format(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (const auto& f : formats) {
        const auto& [format, color_space] = f;
        if (format == VK_FORMAT_B8G8R8A8_SRGB && color_space == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return f;
        }
    }

    return formats[0];
}

static VkPresentModeKHR get_present_mode(const std::vector<VkPresentModeKHR>& present_modes) {
    for (const auto& present_mode : present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D get_surface_extent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
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
