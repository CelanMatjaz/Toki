#include "vulkan_swapchain.h"

#include "core/logging.h"
#include "renderer/vulkan/platform/vulkan_platform.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"

namespace toki {

static VkSurfaceFormatKHR get_surface_format(const std::vector<VkSurfaceFormatKHR>& formats);
static VkPresentModeKHR get_present_mode(const std::vector<VkPresentModeKHR>& present_modes);
static VkExtent2D get_surface_extent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

void VulkanSwapchain::create(Ref<RendererContext> ctx, const Config& config) {
    m_window = reinterpret_cast<GLFWwindow*>(config.window->get_handle());
    m_surface = create_surface(ctx, m_window);

    u32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, m_surface, &format_count, nullptr);
    TK_ASSERT(format_count > 0, "No surface formats found on physical device");
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, m_surface, &format_count, surface_formats.data());
    m_surfaceFormat = get_surface_format(surface_formats);

    u32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical_device, m_surface, &present_mode_count, nullptr);
    TK_ASSERT(present_mode_count > 0, "No present modes found on physical device");
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical_device, m_surface, &present_mode_count, present_modes.data());
    m_presentMode = get_present_mode(present_modes);

    recreate(ctx);

    {
        if (config.command_pool == VK_NULL_HANDLE) {
            VkCommandPoolCreateInfo command_pool_create_info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            command_pool_create_info.queueFamilyIndex = ctx->queue_family_indices.graphics;
            VK_CHECK(vkCreateCommandPool(ctx->device, &command_pool_create_info, ctx->allocation_callbacks, &m_commandPoll), "Could not create dedicatedm_command pool");
        } else {
            m_commandPoll = config.command_pool;
        }

        VkCommandBufferAllocateInfo command_buffer_allocate_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = m_imageCount;
        command_buffer_allocate_info.commandPool = m_commandPoll;

        VkCommandBuffer m_command_buffers[FRAME_COUNT];
        VK_CHECK(vkAllocateCommandBuffers(ctx->device, &command_buffer_allocate_info, m_command_buffers), "Could not allocatem_command buffers")

        for (u32 i = 0; i < m_imageCount; i++) {
            m_frames[i].command.handle = m_command_buffers[i];
            m_frames[i].command.state = vulkan_command_buffer_state::READY;
        }
    }

    {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (uint32_t i = 0; i < m_imageCount; ++i) {
            VK_CHECK(vkCreateFence(ctx->device, &fence_create_info, ctx->allocation_callbacks, &m_frames[i].render_fence), "Could not create render fence");
            VK_CHECK(vkCreateSemaphore(ctx->device, &semaphore_create_info, ctx->allocation_callbacks, &m_frames[i].render_semaphore), "Could not create render semaphore");
            VK_CHECK(vkCreateSemaphore(ctx->device, &semaphore_create_info, ctx->allocation_callbacks, &m_frames[i].present_semaphore), "Could not create present semaphore");
        }
    }
}

void VulkanSwapchain::destroy(Ref<RendererContext> ctx) {
    for (uint32_t i = 0; i < FRAME_COUNT; ++i) {
        vkDestroyFence(ctx->device, m_frames[i].render_fence, ctx->allocation_callbacks);
        m_frames[i].render_fence = VK_NULL_HANDLE;
        vkDestroySemaphore(ctx->device, m_frames[i].render_semaphore, ctx->allocation_callbacks);
        m_frames[i].render_semaphore = VK_NULL_HANDLE;
        vkDestroySemaphore(ctx->device, m_frames[i].present_semaphore, ctx->allocation_callbacks);
        m_frames[i].present_semaphore = VK_NULL_HANDLE;
    }

    for (u32 i = 0; i < m_imageCount; i++) {
        vkDestroyImageView(ctx->device, m_imageViews[i], ctx->allocation_callbacks);
        m_imageViews[i] = VK_NULL_HANDLE;
    }

    vkDestroySwapchainKHR(ctx->device, m_handle, ctx->allocation_callbacks);
    m_handle = VK_NULL_HANDLE;
    vkDestroySurfaceKHR(ctx->instance, m_surface, ctx->allocation_callbacks);
    m_surface = VK_NULL_HANDLE;
}

void VulkanSwapchain::recreate(Ref<RendererContext> ctx) {
    TK_LOG_INFO("(Re)creating swapchain");

    for (u32 i = 0; i < m_imageCount; i++) {
        if (m_imageViews[i] == VK_NULL_HANDLE) {
            continue;
        }
        vkDestroyImageView(ctx->device, m_imageViews[i], ctx->allocation_callbacks);
        m_imageViews[i] = VK_NULL_HANDLE;
    }

    m_currentFrameIndex = 0;

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physical_device, m_surface, &capabilities);

    m_extent = get_surface_extent(m_window, capabilities);
    TK_ASSERT(m_extent.width > 0 && m_extent.height > 0, "Surface extent is not of valid size");

    m_imageCount = std::clamp(static_cast<u32>(FRAME_COUNT), capabilities.minImageCount, capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = m_imageCount;
    swapchain_create_info.imageFormat = m_surfaceFormat.format;
    swapchain_create_info.imageColorSpace = m_surfaceFormat.colorSpace;
    swapchain_create_info.imageExtent = m_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = m_presentMode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = m_handle;

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

    VK_CHECK(vkCreateSwapchainKHR(ctx->device, &swapchain_create_info, ctx->allocation_callbacks, &m_handle), "Could not create swapchain");

    if (swapchain_create_info.oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(ctx->device, swapchain_create_info.oldSwapchain, ctx->allocation_callbacks);
    }

    {
        vkGetSwapchainImagesKHR(ctx->device, m_handle, &m_imageCount, m_images);
        TK_ASSERT(m_imageCount > 0, "No images found for swapchain");

        VkImageViewCreateInfo image_view_create_info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = m_surfaceFormat.format;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.layerCount = 1;

        for (u32 i = 0; i < m_imageCount; i++) {
            image_view_create_info.image = m_images[i];
            VK_CHECK(vkCreateImageView(ctx->device, &image_view_create_info, ctx->allocation_callbacks, &m_imageViews[i]), "Could not createm_image view");
        }
    }

    for (auto& framebuffer : ctx->framebuffers) {
        framebuffer.resize(ctx, VkExtent3D{ m_extent.width, m_extent.height, 1 });
    }
}

b8 VulkanSwapchain::start_recording(Ref<RendererContext> ctx) {
    Frame& frame = m_frames[m_currentFrameIndex];
    VkResult result = vkAcquireNextImageKHR(ctx->device, m_handle, UINT64_MAX, frame.present_semaphore, VK_NULL_HANDLE, &m_currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate(ctx);
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

void VulkanSwapchain::stop_recording(Ref<RendererContext> ctx) {
    Frame& frame = m_frames[m_currentImageIndex];
    vkEndCommandBuffer(frame.command.handle);
    frame.command.state = vulkan_command_buffer_state::RECORDING_STOPPED;
}

void VulkanSwapchain::end_frame(Ref<RendererContext> ctx) {
    m_currentFrameIndex = (m_currentFrameIndex + 1) % m_imageCount;

    VkResult waitFencesResult = vkWaitForFences(ctx->device, 1, &get_current_frame().render_fence, VK_TRUE, UINT64_MAX);
    TK_ASSERT(waitFencesResult == VK_SUCCESS || waitFencesResult == VK_TIMEOUT, "Failed waiting for fences");
}

void VulkanSwapchain::transition_current_frame_image() {
    VulkanImage::transition_layout(get_current_command_buffer(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, m_images[m_currentImageIndex]);
}

VkSwapchainKHR VulkanSwapchain::get_handle() const {
    return m_handle;
}

VkCommandBuffer VulkanSwapchain::get_current_command_buffer() const {
    return m_frames[m_currentFrameIndex].command.handle;
}

VkFormat VulkanSwapchain::get_format() const {
    return m_surfaceFormat.format;
}

VkExtent2D VulkanSwapchain::get_extent() const {
    return m_extent;
}

VkImageView VulkanSwapchain::get_current_frame_image_view() const {
    return m_imageViews[m_currentFrameIndex];
}

Frame& VulkanSwapchain::get_current_frame() {
    return m_frames[m_currentFrameIndex];
}

u32 VulkanSwapchain::get_current_frame_index() const {
    return m_currentFrameIndex;
}

u32 VulkanSwapchain::get_current_image_index() const {
    return m_currentFrameIndex;
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
