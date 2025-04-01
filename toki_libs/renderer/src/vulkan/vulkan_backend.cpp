#include "vulkan_backend.h"

#include <toki/core.h>
#include <vulkan/vulkan.h>

#include "vulkan/vulkan_commands.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_types.h"

#if defined(TK_PLATFORM_WINDOWS)
#include <vulkan/vulkan_win32.h>
#endif

#include "vulkan_platform.h"

#define ASSERT_EXISTS(resource_array, handle, resource)                \
    TK_ASSERT(                                                         \
        handle.valid() && mResources->resource_array.contains(handle), \
        "Handle is not associated with any Vulkan " #resource);

#define ASSERT_BUFFER(handle) ASSERT_EXISTS(buffers, handle, buffer)
#define ASSERT_IMAGE(handle) ASSERT_EXISTS(images, handle, image)
#define ASSERT_SHADER(handle) ASSERT_EXISTS(shaders, handle, shader)
#define ASSERT_FRAMEBUFFER(handle) ASSERT_EXISTS(framebuffers, handle, framebuffer)

#define SWAPCHAIN_IMAGE(swapchain) swapchain.images[swapchain.image_index]

namespace toki {

VulkanBackend::VulkanBackend():
    mAllocator(GB(2)),
    mContext(mAllocator),
    mResources(mAllocator),
    mSettings(mAllocator),
    mFrameAllocator(mAllocator, MB(50)) {
    create_instance();
    device_create();
    resources_initialize();
}

VulkanBackend::~VulkanBackend() {
    resources_cleanup();
}

const Limits& VulkanBackend::limits() const {
    return mContext->limits;
}

const DeviceProperties& VulkanBackend::device_properties() const {
    return mContext->properties;
}

static VkFormat get_format(ColorFormat format_in);
static VkFormat get_depth_format(VkPhysicalDevice physical_device, b8 has_stencil);

void VulkanBackend::create_instance() {
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "Toki";
    application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    application_info.pEngineName = "Toki Engine";
    application_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_3;

    StaticArray<const char*, 2, BumpAllocator> extensions(mFrameAllocator);
    extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
#if defined(TK_PLATFORM_WINDOWS)
    extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(TK_PLATFORM_LINUX)
    extensions[1] = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#endif

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;

#ifndef TK_DIST
    {
        BasicRef<const char*, BumpAllocator> validation_layers(mFrameAllocator.get(), 1);
        validation_layers[0] = "VK_LAYER_KHRONOS_validation";

        b8 layers_supported = true;

        uint32_t layer_count{};
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        BumpRef<VkLayerProperties> layers(mFrameAllocator.get(), layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

        for (u32 i = 0; i < validation_layers.size(); i++) {
            const char* required_layer = validation_layers[i];
            b8 layer_found = false;

            for (u32 i = 0; i < layers.size(); i++) {
                const auto& found_layer = layers[i];
                if (strcmp(required_layer, found_layer.layerName) == 0) {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found) {
                layers_supported = false;
                break;
            }
        }

        TK_ASSERT(layers_supported, "Validation layers not supported");
        instance_create_info.enabledLayerCount = validation_layers.size();
        instance_create_info.ppEnabledLayerNames = validation_layers.data();
    }
#endif

    instance_create_info.enabledExtensionCount = extensions.size();
    instance_create_info.ppEnabledExtensionNames = extensions.data();

    TK_LOG_INFO("Creating new Vulkan instance");
    VK_CHECK(
        vkCreateInstance(&instance_create_info, mContext->allocation_callbacks, &mContext->instance),
        "Could not initialize renderer");
}

void VulkanBackend::find_physical_device(VkSurfaceKHR surface) {
    u32 physical_device_count{};
    vkEnumeratePhysicalDevices(mContext->instance, &physical_device_count, nullptr);
    TK_ASSERT(physical_device_count > 0, "No GPUs found");
    BumpRef<VkPhysicalDevice> physical_devices(mFrameAllocator.get(), physical_device_count);
    vkEnumeratePhysicalDevices(mContext->instance, &physical_device_count, physical_devices);

    auto rate_physical_device_suitability = []([[maybe_unused]] VkPhysicalDevice physical_device,
                                               [[maybe_unused]] VkPhysicalDeviceProperties properties,
                                               [[maybe_unused]] VkPhysicalDeviceFeatures features) {
        u32 score = 0;

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }

        return score;
    };

    u32 best_score = 0;
    VkPhysicalDeviceProperties device_properties{};
    VkPhysicalDeviceFeatures device_features{};

    for (u32 i = 0; i < physical_device_count; i++) {
        VkPhysicalDevice physical_device = physical_devices[i];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_device, &features);

        u32 score = rate_physical_device_suitability(physical_device, properties, features);
        if (score > best_score) {
            mContext->physical_device = physical_device;
            mContext->physical_device_properties = properties;
            device_properties = properties;
            device_features = features;
        }
    }

    mContext->limits.max_framebuffer_width = device_properties.limits.maxFramebufferWidth;
    mContext->limits.max_framebuffer_height = device_properties.limits.maxFramebufferHeight;
    mContext->limits.max_push_constant_size = device_properties.limits.maxPushConstantsSize;
    mContext->limits.max_color_attachments = device_properties.limits.maxColorAttachments;

    mContext->properties.depth_format = get_depth_format(mContext->physical_device, false);
    mContext->properties.depth_stencil_format = get_depth_format(mContext->physical_device, true);

    u32 queue_family_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(mContext->physical_device, &queue_family_count, nullptr);
    BumpRef<VkQueueFamilyProperties> queue_families(mFrameAllocator.get(), queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(mContext->physical_device, &queue_family_count, queue_families);

    for (u32 i = 0; i < queue_family_count; i++) {
        VkBool32 supports_present = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(mContext->physical_device, i, surface, &supports_present);
        if (supports_present && mContext->present_queue.family_index == -1) {
            mContext->present_queue.family_index = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && mContext->graphics_queue.family_index == -1) {
            mContext->graphics_queue.family_index = i;
        }
    }

    TK_ASSERT(mContext->present_queue.family_index != 1, "No queue family that supports presenting found");
    TK_ASSERT(mContext->graphics_queue.family_index != 1, "No queue family that supports graphics found");
}

void VulkanBackend::device_create() {
    auto handle = window_create("", 0, 0);
    VkSurfaceKHR surface = vulkan_surface_create(mContext->instance, mContext->allocation_callbacks, handle);

    find_physical_device(surface);

    f32 queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_infos[2]{};
    queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[0].queueFamilyIndex = mContext->graphics_queue.family_index;
    queue_create_infos[0].queueCount = 1;
    queue_create_infos[0].pQueuePriorities = &queue_priority;
    queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[1].queueFamilyIndex = mContext->present_queue.family_index;
    queue_create_infos[1].queueCount = 1;
    queue_create_infos[1].pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures features{};
    features.fillModeNonSolid = VK_TRUE;
    features.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature{};
    dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamic_rendering_feature.dynamicRendering = VK_TRUE;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &dynamic_rendering_feature;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.enabledExtensionCount = sizeof(vulkan_extensions) / sizeof(const char*);
    device_create_info.ppEnabledExtensionNames = vulkan_extensions;
    device_create_info.pEnabledFeatures = &features;

    TK_LOG_INFO("Creating new Vulkan device");
    VK_CHECK(
        vkCreateDevice(
            mContext->physical_device, &device_create_info, mContext->allocation_callbacks, &mContext->device),
        "Could not create Vulkan device");

    vkGetDeviceQueue(mContext->device, mContext->graphics_queue.family_index, 0, &mContext->graphics_queue.handle);
    vkGetDeviceQueue(mContext->device, mContext->present_queue.family_index, 0, &mContext->present_queue.handle);

    vkDestroySurfaceKHR(mContext->instance, surface, mContext->allocation_callbacks);
    window_destroy(handle);
}

static VkSurfaceFormatKHR get_surface_format(VkSurfaceFormatKHR* formats, u32 format_count) {
    for (u32 i = 0; i < format_count; i++) {
        const auto& [format, color_space] = formats[i];
        if (format == VK_FORMAT_B8G8R8A8_SRGB && color_space == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }

    return formats[0];
}

static VkPresentModeKHR get_disabled_vsync_present_mode(VkPresentModeKHR* present_modes, u32 mode_count) {
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < mode_count; i++) {
        switch (present_modes[i]) {
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                return VK_PRESENT_MODE_IMMEDIATE_KHR;
            case VK_PRESENT_MODE_MAILBOX_KHR:
                present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                if (present_mode != VK_PRESENT_MODE_MAILBOX_KHR) {
                    present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
                }
                break;
            case VK_PRESENT_MODE_FIFO_KHR:
                break;
            default:
                UNREACHABLE;
        }
    }

    return present_mode;
}

static VkExtent2D get_surface_extent(VkSurfaceCapabilitiesKHR* capabilities, NativeWindowHandle handle) {
    if (capabilities->currentExtent.width != (0UL - 1)) {
        return capabilities->currentExtent;
    }

    auto dimensions = window_get_dimensions(handle);

    VkExtent2D extent{ dimensions.x, dimensions.y };
    extent.width = clamp(extent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
    extent.height = clamp(extent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

    return extent;
}

void VulkanBackend::swapchain_create(NativeWindowHandle handle) {
    TK_LOG_INFO("Creating swapchain");

    Swapchain& swapchain = mResources->swapchain;
    swapchain.window_handle = handle;
    swapchain.can_render = true;
    VkSurfaceKHR surface = swapchain.surface =
        vulkan_surface_create(mContext->instance, mContext->allocation_callbacks, handle);

    // Query swapchain surface formats
    {
        u32 format_count{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(mContext->physical_device, surface, &format_count, nullptr);
        TK_ASSERT(format_count > 0, "No surface formats found on physical device");

        VkSurfaceFormatKHR* surface_formats = mFrameAllocator->allocate<VkSurfaceFormatKHR>(format_count);

        vkGetPhysicalDeviceSurfaceFormatsKHR(mContext->physical_device, surface, &format_count, surface_formats);
        swapchain.surface_format = get_surface_format(surface_formats, format_count);
    }

    // Query non vsync fallback present mode
    {
        u32 present_mode_count{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(mContext->physical_device, surface, &present_mode_count, nullptr);
        TK_ASSERT(present_mode_count > 0, "No present modes found on physical device");

        VkPresentModeKHR* present_modes = mFrameAllocator->allocate<VkPresentModeKHR>(present_mode_count);

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            mContext->physical_device, surface, &present_mode_count, present_modes);
        swapchain.vsync_disabled_present_mode = get_disabled_vsync_present_mode(present_modes, present_mode_count);
    }

    // Query swapchain image count
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mContext->physical_device, surface, &capabilities);
        swapchain.image_count = clamp(MAX_FRAMES_IN_FLIGHT, capabilities.minImageCount, capabilities.maxImageCount);
    }

    // Create mage available semaphore
    {
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_CHECK(
                vkCreateSemaphore(
                    mContext->device,
                    &semaphore_create_info,
                    mContext->allocation_callbacks,
                    &swapchain.image_available_semaphores[i]),
                "Could not create render semaphore");
        }
    }

    swapchain_recreate(swapchain);

    // window->get_event_handler().bind_event(EventType::WindowResize, this, [this, &swapchain](void*, void*, Event&) {
    //     if (!swapchain.can_render) {
    //         return;
    //     }
    //
    //     this->wait_for_resources();
    //     TK_LOG_INFO("Recreating swapchain");
    //     this->recreate_swapchain(&swapchain);
    // });

    // window->get_event_handler().bind_event(EventType::WindowRestore, this, [&swapchain](void*, void*, Event&) {
    //     swapchain.can_render = true;
    // });

    // window->get_event_handler().bind_event(EventType::WindowMinimize, this, [&swapchain](void*, void*, Event&) {
    //     swapchain.can_render = false;
    // });
}

void VulkanBackend::swapchain_recreate(Swapchain& swapchain) {
    if (swapchain.image_views.size() > 0) {
        VkImageView* image_views = swapchain.image_views.data();
        for (u32 i = 0; i < swapchain.image_views.size(); i++) {
            vkDestroyImageView(mContext->device, image_views[i], mContext->allocation_callbacks);
        }
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mContext->physical_device, swapchain.surface, &capabilities);

    swapchain.extent = get_surface_extent(&capabilities, swapchain.window_handle);
    TK_ASSERT(swapchain.extent.width > 0 && swapchain.extent.height > 0, "Surface extent is not of valid size");

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
    swapchain_create_info.presentMode =
        mSettings->vsync_enabled ? VK_PRESENT_MODE_FIFO_KHR : swapchain.vsync_disabled_present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = swapchain.swapchain;

    u32 queue_family_indices[] = { static_cast<u32>(mContext->present_queue.family_index),
                                   static_cast<u32>(mContext->graphics_queue.family_index) };

    if (queue_family_indices[0] != queue_family_indices[1]) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
    }

    VK_CHECK(
        vkCreateSwapchainKHR(
            mContext->device, &swapchain_create_info, mContext->allocation_callbacks, &swapchain.swapchain),
        "Could not create swapchain");

    if (swapchain_create_info.oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(mContext->device, swapchain_create_info.oldSwapchain, mContext->allocation_callbacks);
    }

    {
        vkGetSwapchainImagesKHR(mContext->device, swapchain.swapchain, &swapchain.image_count, nullptr);
        swapchain.images = DynamicArray<VkImage>(mAllocator, swapchain.image_count);
        VkImage* swapchain_images = swapchain.images.data();
        vkGetSwapchainImagesKHR(mContext->device, swapchain.swapchain, &swapchain.image_count, swapchain_images);
        TK_ASSERT(swapchain.image_count > 0, "No images found for swapchain");

        if (swapchain.image_views.size() == 0) {
            swapchain.image_views = DynamicArray<VkImageView>(mAllocator, swapchain.image_count);
        }

        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = swapchain.surface_format.format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        for (u32 i = 0; i < swapchain.image_count; i++) {
            image_view_create_info.image = swapchain_images[i];
            VK_CHECK(
                vkCreateImageView(
                    mContext->device,
                    &image_view_create_info,
                    mContext->allocation_callbacks,
                    &swapchain.image_views[i]),
                "Could not create image view");
        }
    }
}

void VulkanBackend::swapchain_destroy() {
    Swapchain& swapchain = mResources->swapchain;

    for (u32 i = 0; i < swapchain.image_count; i++) {
        vkDestroyImageView(mContext->device, swapchain.image_views[i], mContext->allocation_callbacks);
    }

    vkDestroySwapchainKHR(mContext->device, swapchain.swapchain, mContext->allocation_callbacks);
    vkDestroySurfaceKHR(mContext->instance, swapchain.surface, mContext->allocation_callbacks);
}

Handle VulkanBackend::framebuffer_create(const FramebufferConfig& config) {
    InternalFramebuffer framebuffer{};
    framebuffer.has_depth = config.has_depth_attachment;
    framebuffer.has_stencil = config.has_stencil_attachment;
    framebuffer.attachment_count = config.color_format_count;
    framebuffer.image_color_format = config.color_format;

    TK_ASSERT(
        config.color_format_count <= limits().max_color_attachments,
        "Maximum %ul color attachments supported",
        limits().max_color_attachments);

    framebuffer.color_image = BasicRef<InternalImage>(mAllocator);

    *framebuffer.color_image = image_internal_create(
        config.image_width,
        config.image_height,
        config.color_format_count,
        get_format(config.color_format),
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageAspectFlags depthStenctilAspectFlags = 0;

    if (config.has_depth_attachment) {
        depthStenctilAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    if (config.has_stencil_attachment) {
        depthStenctilAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    if (depthStenctilAspectFlags > 0) {
        framebuffer.depth_stencil_image = BasicRef<InternalImage>(mAllocator);
        *framebuffer.depth_stencil_image = image_internal_create(
            config.image_width,
            config.image_height,
            1,
            get_depth_format(
                mContext->physical_device,
                (depthStenctilAspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT) == VK_IMAGE_ASPECT_STENCIL_BIT),
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            depthStenctilAspectFlags);
    }

    return mResources->framebuffers.insert(move(framebuffer));
}

void VulkanBackend::framebuffer_destroy(Handle& framebuffer_handle) {
    ASSERT_FRAMEBUFFER(framebuffer_handle);
    InternalFramebuffer& framebuffer = mResources->framebuffers[framebuffer_handle];
    image_internal_destroy(framebuffer.color_image);
    if (framebuffer.depth_stencil_image) {
        image_internal_destroy(*framebuffer.depth_stencil_image.data());
    }
}

Handle VulkanBackend::buffer_create(const BufferConfig& config) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    switch (config.type) {
        case BufferType::VERTEX:
            usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case BufferType::INDEX:
            usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        default:
            UNREACHABLE;
    }

    return mResources->buffers.insert(
        move(buffer_internal_create(config.size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
}

void VulkanBackend::buffer_destroy(Handle& buffer_handle) {
    ASSERT_BUFFER(buffer_handle)
    InternalBuffer& internal_buffer = mResources->buffers[buffer_handle];
    buffer_internal_destroy(internal_buffer);
    mResources->buffers.invalidate(buffer_handle);
}

void* VulkanBackend::buffer_map_memory(VkDeviceMemory memory, u32 offset, u32 size) {
    void* data{};
    VK_CHECK(vkMapMemory(mContext->device, memory, offset, size, 0, &data), "Could not map buffer memory");
    return data;
}

void VulkanBackend::buffer_unmap_memory(VkDeviceMemory memory) {
    vkUnmapMemory(mContext->device, memory);
}

void VulkanBackend::buffer_flush(Buffer& buffer) {
    TK_ASSERT(mResources->buffers.contains(buffer), "Buffer with provided handle does not exist");
    InternalBuffer& internal_buffer = mResources->buffers[buffer];

    if ((internal_buffer.memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
        VkMappedMemoryRange mapped_memory_range{};
        mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mapped_memory_range.memory = internal_buffer.memory;
        mapped_memory_range.offset = 0;
        mapped_memory_range.size = internal_buffer.size;
        VK_CHECK(vkFlushMappedMemoryRanges(mContext->device, 1, &mapped_memory_range), "Could not flush buffer memory");
    }
}

void VulkanBackend::buffer_set_data(const Buffer& buffer, u32 size, void* data) {
    TK_ASSERT(mResources->buffers.contains(buffer.handle), "Buffer with provided handle does not exist");
    InternalBuffer& staging_buffer = mResources->staging_buffer;

    // Copy to staging buffer
    void* mapped_data = buffer_map_memory(staging_buffer.memory, mResources->staging_buffer_offset, size);
    toki::memcpy(data, mapped_data, size);
    buffer_unmap_memory(staging_buffer.memory);
    mapped_data = nullptr;

    InternalBuffer& internal_buffer = mResources->buffers[buffer.handle];

    // Copy to uploaded buffer
    buffer_copy_data(internal_buffer.buffer, staging_buffer.buffer, size, 0, mResources->staging_buffer_offset);

    mResources->staging_buffer_offset += size;
}

void VulkanBackend::buffer_copy_data(VkBuffer dst, VkBuffer src, u32 size, u32 dst_offset, u32 src_offset) {
    VkCommandBuffer cmd = start_single_use_command_buffer();

    VkBufferCopy buffer_copy{};
    buffer_copy.srcOffset = src_offset;
    buffer_copy.dstOffset = dst_offset;
    buffer_copy.size = size;
    vkCmdCopyBuffer(cmd, src, dst, 1, &buffer_copy);

    submit_single_use_command_buffer(cmd);
}

Handle VulkanBackend::image_create(const TextureConfig& config) {
    InternalImage new_image = image_internal_create(
        config.width,
        config.height,
        1,
        get_format(config.format),
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT);

    return mResources->images.insert(move(new_image));
}

void VulkanBackend::image_destroy(Handle& texture_handle) {
    ASSERT_IMAGE(texture_handle);
    mResources->images.invalidate(texture_handle);
}

Handle VulkanBackend::shader_create(const Framebuffer& framebuffer, const ShaderConfig& config) {
    InternalShader new_shader(mAllocator);
    new_shader.type = config.type;
    new_shader.framebuffer_handle = framebuffer.handle;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pSetLayouts = nullptr;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;
    pipeline_layout_create_info.pushConstantRangeCount = 0;

    VK_CHECK(
        vkCreatePipelineLayout(
            mContext->device,
            &pipeline_layout_create_info,
            mContext->allocation_callbacks,
            &new_shader.pipeline_layout),
        "Could not create pipeline layout");

    return mResources->shaders.insert(move(new_shader));
}

void VulkanBackend::shader_destroy(Handle& shader_handle) {
    ASSERT_SHADER(shader_handle);
    InternalShader& shader = mResources->shaders[shader_handle];
    for (u32 i = 0; i < shader.pipelines.size(); i++) {
        vkDestroyPipeline(mContext->device, shader.pipelines[i].pipeline, mContext->allocation_callbacks);
    }
}

Handle VulkanBackend::shader_variant_create(Shader& _shader, const ShaderVariantConfig& config) {
    ASSERT_SHADER(_shader.handle);
    InternalShader& shader = mResources->shaders[_shader.handle];
    ASSERT_FRAMEBUFFER(shader.framebuffer_handle);
    InternalFramebuffer& framebuffer = mResources->framebuffers[shader.framebuffer_handle];

    pipeline_internal_create(framebuffer, config, shader.pipeline_layout);

    return {};
}

// PipelineResources VulkanBackend::create_pipeline_resources(const std::vector<configs::Shader>& _) {
//     PipelineResources resources{};
//
//     u32 push_constant_count = 0;
//     VkPushConstantRange* push_constants = m_frameAllocator->allocate_aligned<VkPushConstantRange>(16);
//     u32 descriptor_set_layout_count = 0;
//     VkDescriptorSetLayout* descriptor_set_layouts = m_frameAllocator->allocate_aligned<VkDescriptorSetLayout>(16);
//
//     VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
//     pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//     pipeline_layout_create_info.setLayoutCount = descriptor_set_layout_count;
//     pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts;
//     pipeline_layout_create_info.pushConstantRangeCount = push_constant_count;
//     pipeline_layout_create_info.pPushConstantRanges = push_constants;
//
//     VK_CHECK(
//         vkCreatePipelineLayout(
//             mContext->device, &pipeline_layout_create_info, mContext->allocation_callbacks,
//             &resources.pipeline_layout),
//         "Could not create pipeline layout");
//
//     return resources;
// }

InternalPipeline VulkanBackend::pipeline_internal_create(
    const InternalFramebuffer& framebuffer, const ShaderVariantConfig& config, VkPipelineLayout pipeline_layout) {
    InternalPipeline pipeline{};

    TK_ASSERT(
        config.sources[static_cast<u32>(ShaderStage::VERTEX)].source_path.valid(),
        "Graphics pipeline requires vertex shader source");
    TK_ASSERT(
        config.sources[static_cast<u32>(ShaderStage::FRAGMENT)].source_path.valid(),
        "Graphics pipeline requires fragment shader source");

    VkPipelineShaderStageCreateInfo default_pipeline_shader_stage_create_info{};
    default_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    default_pipeline_shader_stage_create_info.pName = "main";

    u32 max_shader_stages = static_cast<u32>(ShaderStage::SHADER_STAGE_COUNT);

    DynamicArray<VkPipelineShaderStageCreateInfo, BumpAllocator> shader_stage_create_infos(
        mFrameAllocator, max_shader_stages);

    shader_stage_create_infos[0] = default_pipeline_shader_stage_create_info;
    shader_stage_create_infos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stage_create_infos[0].module = create_shader_module(
        ShaderStage::VERTEX, config.sources[0].source_path.data(), config.sources[0].source_path.size());

    shader_stage_create_infos[1] = default_pipeline_shader_stage_create_info;
    shader_stage_create_infos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stage_create_infos[1].module = create_shader_module(
        ShaderStage::VERTEX, config.sources[1].source_path.data(), config.sources[1].source_path.size());

    DynamicArray<VkVertexInputBindingDescription, BumpAllocator> vertex_binding_descriptions(
        mFrameAllocator, config.binding_count);
    for (u32 i = 0; i < config.binding_count; i++) {
        vertex_binding_descriptions[i].binding = config.vertex_bindings[i].binding;
        vertex_binding_descriptions[i].stride = config.vertex_bindings[i].stride;

        switch (config.vertex_bindings[i].inputRate) {
            case VertexInputRate::VERTEX:
                vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                break;
            case VertexInputRate::INSTANCE:
                vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                break;
        }
    }

    DynamicArray<VkVertexInputAttributeDescription, BumpAllocator> vertex_attribute_descriptions(
        mFrameAllocator, config.attribute_count);
    for (u32 i = 0; i < config.attribute_count; i++) {
        vertex_attribute_descriptions[i].binding = config.vertex_attributes[i].binding;
        vertex_attribute_descriptions[i].offset = config.vertex_attributes[i].offset;
        vertex_attribute_descriptions[i].location = config.vertex_attributes[i].location;

        switch (config.vertex_attributes[i].format) {
            case VertexFormat::FLOAT1:
                vertex_attribute_descriptions[i].format = VK_FORMAT_R32_SFLOAT;
                break;
            case VertexFormat::FLOAT2:
                vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case VertexFormat::FLOAT3:
                vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case VertexFormat::FLOAT4:
                vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            default:
                UNREACHABLE;
        }
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount = vertex_binding_descriptions.size();
    vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
    vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

    switch (config.primitive_topology) {
        case PrimitiveTopology::POINT_LIST:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case PrimitiveTopology::LINE_LIST:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case PrimitiveTopology::LINE_STRIP:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case PrimitiveTopology::TRIANGLE_LIST:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case PrimitiveTopology::TRIANGLE_STRIP:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case PrimitiveTopology::TRIANGLE_FAN:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            break;
        case PrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::PATH_LIST:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            break;
    }

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.depthClampEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.lineWidth = 1.0f;
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    rasterization_state_create_info.depthBiasClamp = 0.0f;
    rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

    switch (config.front_face) {
        case FrontFace::CLOCKWISE:
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
            break;
        case FrontFace::COUNTER_CLOCKWISE:
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            break;
    }

    switch (config.polygon_mode) {
        case PolygonMode::FILL:
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            break;
        case PolygonMode::LINE:
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_LINE;
            break;
        case PolygonMode::POINT:
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_POINT;
            break;
    }

    switch (config.cull_mode) {
        case CullMode::NONE:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
            break;
        case CullMode::FRONT:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_BIT;
            break;
        case CullMode::BACK:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case CullMode::BOTH:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
            break;
    }

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.sampleShadingEnable = VK_FALSE;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_create_info.minSampleShading = 1.0f;
    multisample_state_create_info.pSampleMask = nullptr;
    multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_create_info.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
    depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state_create_info.depthTestEnable = VK_FALSE;
    if (config.depth_test_config.valid()) {
        depth_stencil_state_create_info.depthTestEnable = config.depth_test_config.valid() ? VK_TRUE : VK_FALSE;
        depth_stencil_state_create_info.depthWriteEnable = config.depth_test_config->write_enable ? VK_TRUE : VK_FALSE;
        depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_state_create_info.minDepthBounds = 0.0f;
        depth_stencil_state_create_info.maxDepthBounds = 1.0f;
        depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
        depth_stencil_state_create_info.front = {};
        depth_stencil_state_create_info.back = {};

        switch (config.depth_test_config->compare_operation) {
            case CompareOp::NEVER:
                depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_NEVER;
                break;
            case CompareOp::LESS:
                depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
                break;
            case CompareOp::EQUAL:
                depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_EQUAL;
                break;
            case CompareOp::LESS_OR_EQUAL:
                depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                break;
            case CompareOp::GREATER:
                depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_GREATER;
                break;
            case CompareOp::NOT_EQUAL:
                depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
                break;
            case CompareOp::GREATER_OR_EQUAL:
                depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
                break;
            case CompareOp::ALWAYS:
                depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_ALWAYS;
                break;
        }
    }

    VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

    DynamicArray<VkPipelineColorBlendAttachmentState, BumpAllocator> color_blend_attachment_states(
        mFrameAllocator, framebuffer.attachment_count, move(color_blend_attachment_state));

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_create_info.attachmentCount = color_blend_attachment_states.size();
    color_blend_state_create_info.pAttachments = color_blend_attachment_states.data();
    color_blend_state_create_info.blendConstants[0] = 1.0f;
    color_blend_state_create_info.blendConstants[1] = 1.0f;
    color_blend_state_create_info.blendConstants[2] = 1.0f;
    color_blend_state_create_info.blendConstants[3] = 1.0f;

    constexpr VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = std::size(dynamic_states);
    dynamic_state_create_info.pDynamicStates = dynamic_states;

    VkViewport viewport{};
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    BumpRef<VkFormat> formats(mFrameAllocator, framebuffer.attachment_count);
    for (u32 i = 0; i < formats.size(); i++) {
        formats[i] = get_format(framebuffer.image_color_format);
    }

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{};
    pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipeline_rendering_create_info.colorAttachmentCount = framebuffer.attachment_count;
    pipeline_rendering_create_info.pColorAttachmentFormats = formats;
    if (framebuffer.has_depth) {
        pipeline_rendering_create_info.depthAttachmentFormat = framebuffer.depth_stencil_image->format;
    }
    if (framebuffer.has_stencil) {
        pipeline_rendering_create_info.stencilAttachmentFormat = framebuffer.depth_stencil_image->format;
    }

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.pNext = &pipeline_rendering_create_info;
    graphics_pipeline_create_info.stageCount = shader_stage_create_infos.size();
    graphics_pipeline_create_info.pStages = shader_stage_create_infos.data();
    graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    graphics_pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
    graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
    graphics_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    graphics_pipeline_create_info.renderPass = nullptr;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.layout = pipeline_layout;

    TK_LOG_INFO("Creating new graphics pipeline");
    VK_CHECK(
        vkCreateGraphicsPipelines(
            mContext->device,
            VK_NULL_HANDLE,
            1,
            &graphics_pipeline_create_info,
            mContext->allocation_callbacks,
            &pipeline.pipeline),
        "Could not create graphics pipeline");

    for (u32 i = 0; i < shader_stage_create_infos.size(); i++) {
        vkDestroyShaderModule(mContext->device, shader_stage_create_infos[i].module, mContext->allocation_callbacks);
    }

    return pipeline;
}

void VulkanBackend::pipeline_internal_destroy(InternalPipeline& pipeline) {
    vkDestroyPipeline(mContext->device, pipeline.pipeline, mContext->allocation_callbacks);
}

/*
void VulkanBackend::reflect_shader(
    ShaderStage stage,
    std::vector<u32>& binary,
    DescriptorBindings& bindings,
    std::vector<VkPushConstantRange>& push_constants) {
    spirv_cross::Compiler compiler(binary);
    const auto& resources = compiler.get_shader_resources();

    VkShaderStageFlags shader_stage{};
    switch (stage) {
        case ShaderStage::Vertex:
            shader_stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case ShaderStage::Fragment:
            shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        default:
            UNREACHABLE;
    }

    for (u32 i = 0; i < resources.push_constant_buffers.size(); i++) {
        auto& element_type = compiler.get_type(resources.push_constant_buffers[i].base_type_id);
        if (push_constants.size() == 1) {
            push_constants[i].stageFlags |= shader_stage;
        } else {
            VkPushConstantRange push_constant{};
            push_constant.size = compiler.get_declared_struct_size(element_type);
            push_constant.offset =
                compiler.get_decoration(resources.push_constant_buffers[i].id, spv::DecorationOffset);
            push_constant.stageFlags = shader_stage;
            push_constants.emplace_back(push_constant);
        }
    }

    struct ResourceDescriptorType {
        VkDescriptorType type;
        spirv_cross::SmallVector<spirv_cross::Resource> resources;
    };

    // Supported descriptor types
    std::vector<ResourceDescriptorType> descriptor_type_arrays = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, resources.uniform_buffers },
        { VK_DESCRIPTOR_TYPE_SAMPLER, resources.separate_samplers },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, resources.separate_images },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.sampled_images },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resources.storage_buffers },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, resources.storage_images },
    };

    for (const auto& [descriptor_type, descriptor_array] : descriptor_type_arrays) {
        for (u32 descriptor_index = 0; descriptor_index < descriptor_array.size(); descriptor_index++) {
            auto& element_type = compiler.get_type(descriptor_array[descriptor_index].base_type_id);

            u32 set_index =
                compiler.get_decoration(descriptor_array[descriptor_index].id, spv::DecorationDescriptorSet);
            TK_ASSERT(
                set_index <= MAX_DESCRIPTOR_SET_COUNT,
                "A maximum of {} descriptor sets supported (indices 0 - {}), found index is {}",
                MAX_DESCRIPTOR_SET_COUNT,
                MAX_DESCRIPTOR_SET_COUNT - 1,
                set_index);
            TK_ASSERT(
                bindings.binding_counts[set_index] <= MAX_DESCRIPTOR_BINDING_COUNT,
                "A maximum of {} descriptor set bindings supported",
                MAX_DESCRIPTOR_BINDING_COUNT);

            u32 binding_index = compiler.get_decoration(descriptor_array[descriptor_index].id, spv::DecorationBinding);

            bindings.bindings[binding_index][set_index].binding = binding_index;
            bindings.bindings[binding_index][set_index].descriptorType = descriptor_type;
            bindings.bindings[binding_index][set_index].stageFlags |= shader_stage;
            bindings.bindings[binding_index][set_index].descriptorCount =
                element_type.array.size() == 0 ? 1 : element_type.array[0];

            bindings.binding_counts[set_index]++;
        }
    }
}*/

void VulkanBackend::swapchain_prepare_frame(Swapchain& swapchain) {
    VkResult result = vkAcquireNextImageKHR(
        mContext->device,
        swapchain.swapchain,
        UINT64_MAX,
        swapchain.image_available_semaphores[mInFlightFrameIndex],
        VK_NULL_HANDLE,
        &swapchain.image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapchain_recreate(swapchain);
        return;
    } else {
        TK_ASSERT(
            result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR || result == VK_NOT_READY,
            "Could not acquire swapchain image");
    }
}

void VulkanBackend::swapchain_reset_frame(Swapchain& swapchain) {
    swapchain.is_recording_commands = false;
}

VkCommandBuffer VulkanBackend::start_single_use_command_buffer() {
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandPool = mResources->extra_command_pools[0];
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(mContext->device, &command_buffer_allocate_info, &command_buffer);

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info), "Could not begin command buffer");

    return command_buffer;
}

void VulkanBackend::submit_single_use_command_buffer(VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd;

    vkQueueSubmit(mContext->graphics_queue.handle, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(mContext->graphics_queue.handle);
}

FrameData* VulkanBackend::get_current_frame() {
    return &mFrames[mInFlightFrameIndex];
}

CommandBuffers& VulkanBackend::get_current_command_buffers() {
    return mResources->command_buffers;
}

void VulkanBackend::transition_framebuffer_images(VkCommandBuffer cmd, InternalFramebuffer* framebuffer) {
    StaticArray<VkImageMemoryBarrier, 2, BumpAllocator> image_memory_barriers(mFrameAllocator);
    u32 memory_barrier_count = 1;

    VkImageMemoryBarrier& color_image_memory_barrier = image_memory_barriers[0];
    color_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    color_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_image_memory_barrier.srcQueueFamilyIndex = mContext->graphics_queue.family_index;
    color_image_memory_barrier.dstQueueFamilyIndex = mContext->graphics_queue.family_index;
    color_image_memory_barrier.subresourceRange.aspectMask = framebuffer->color_image->aspect_flags;
    color_image_memory_barrier.subresourceRange.baseMipLevel = 0;
    color_image_memory_barrier.subresourceRange.levelCount = 1;
    color_image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    color_image_memory_barrier.subresourceRange.layerCount = framebuffer->color_image->image_views.size();

    if (framebuffer->depth_stencil_image) {
        memory_barrier_count++;

        VkImageMemoryBarrier& depth_stencil_image_memory_barrier = image_memory_barriers[1];
        depth_stencil_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        depth_stencil_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_stencil_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth_stencil_image_memory_barrier.srcQueueFamilyIndex = mContext->graphics_queue.family_index;
        depth_stencil_image_memory_barrier.dstQueueFamilyIndex = mContext->graphics_queue.family_index;
        depth_stencil_image_memory_barrier.subresourceRange.aspectMask = framebuffer->depth_stencil_image->aspect_flags;
        depth_stencil_image_memory_barrier.subresourceRange.baseMipLevel = 0;
        depth_stencil_image_memory_barrier.subresourceRange.levelCount = 1;
        depth_stencil_image_memory_barrier.subresourceRange.baseArrayLayer = 0;
        depth_stencil_image_memory_barrier.subresourceRange.layerCount = 1;

        if (framebuffer->has_stencil) {
            depth_stencil_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
    }

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        memory_barrier_count,
        image_memory_barriers.data());
}

void VulkanBackend::transition_swapchain_image(VkCommandBuffer cmd, Swapchain& swapchain) {
    VkImageMemoryBarrier image_memory_barrier{};
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    image_memory_barrier.srcQueueFamilyIndex = mContext->graphics_queue.family_index;
    image_memory_barrier.dstQueueFamilyIndex = mContext->graphics_queue.family_index;
    image_memory_barrier.image = SWAPCHAIN_IMAGE(swapchain);
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.levelCount = 1;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &image_memory_barrier);
}

void VulkanBackend::transition_image_layout(const TransitionLayoutConfig& config, InternalImage* image) {
    VkCommandBuffer cmd = start_single_use_command_buffer();
    transition_image_layout(cmd, config, image);

    submit_single_use_command_buffer(cmd);
}

void VulkanBackend::transition_image_layout(
    VkCommandBuffer cmd, const TransitionLayoutConfig& config, VkImageAspectFlags aspect_flags, VkImage image) {
    VkImageMemoryBarrier image_memory_barrier =
        create_image_memory_barrier(config.old_layout, config.new_layout, aspect_flags);
    image_memory_barrier.image = image;
    vkCmdPipelineBarrier(cmd, config.src_stage, config.dst_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

void VulkanBackend::transition_image_layout(
    VkCommandBuffer cmd, const TransitionLayoutConfig& config, InternalImage* image) {
    transition_image_layout(cmd, config, image->aspect_flags, image->image);
}

void VulkanBackend::transition_image_layouts(
    VkCommandBuffer cmd,
    const TransitionLayoutConfig& config,
    VkImageAspectFlags aspect_flags,
    VkImage* images,
    u32 image_count) {
    VkImageMemoryBarrier image_memory_barrier =
        create_image_memory_barrier(config.old_layout, config.new_layout, aspect_flags);

    DynamicArray<VkImageMemoryBarrier, BumpAllocator> image_memory_barriers(mFrameAllocator, image_count);
    for (u32 i = 0; i < image_count; i++) {
        image_memory_barriers[i] = image_memory_barrier;
        image_memory_barriers[i].image = images[i];
    }

    vkCmdPipelineBarrier(
        cmd,
        config.src_stage,
        config.dst_stage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        image_memory_barriers.size(),
        image_memory_barriers.data());
}

void VulkanBackend::resources_wait() {
    vkDeviceWaitIdle(mContext->device);
}

void VulkanBackend::prepare_frame_resources() {
    FrameData* frame = get_current_frame();

    VkResult result = vkWaitForFences(mContext->device, 1, &frame->render_fence, VK_TRUE, UINT64_MAX);
    TK_ASSERT(result == VK_SUCCESS || result == VK_TIMEOUT, "Failed waiting for fences");

    swapchain_prepare_frame(mResources->swapchain);

    VK_CHECK(vkResetFences(mContext->device, 1, &frame->render_fence), "Could not reset render fence");

    {
        CommandBuffers& command_buffers = get_current_command_buffers();
        for (u32 i = 0; i < command_buffers.used_count; i++) {
            vkResetCommandBuffer(command_buffers.array[i], 0);
        }
    }
}

void VulkanBackend::submit_commands() {
    CommandBuffers& command_buffers = get_current_command_buffers();
    for (u32 i = 0; i < command_buffers.used_count; i++) {
        vkEndCommandBuffer(command_buffers.array[i]);
    }

    submit_frame_command_buffers();
}

void VulkanBackend::cleanup_frame_resources() {
    swapchain_reset_frame(mResources->swapchain);

    mInFlightFrameIndex = (mInFlightFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    mFrameAllocator.swap();
    mFrameAllocator->clear();

    mResources->staging_buffer_offset = 0;
}

void VulkanBackend::present() {
    FrameData* frame = get_current_frame();
    VkSemaphore wait_semaphores[] = { frame->present_semaphore };

    u32 swapchain_count = 1;

    DynamicArray<VkSwapchainKHR, BumpAllocator> swapchain_handles(mFrameAllocator, swapchain_count);
    DynamicArray<u32, BumpAllocator> swapchain_image_indices(mFrameAllocator, swapchain_count);
    DynamicArray<Swapchain*, BumpAllocator> swapchains(mFrameAllocator, swapchain_count);

    swapchain_count = 0;

    Swapchain& swapchain = mResources->swapchain;
    if (!swapchain.can_render) {}

    swapchain_handles[swapchain_count] = swapchain.swapchain;
    swapchain_image_indices[swapchain_count] = swapchain.image_index;
    swapchains[swapchain_count] = &swapchain;
    swapchain_count++;

    DynamicArray<VkResult, BumpAllocator> results(mFrameAllocator, swapchain_count);

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = wait_semaphores;
    present_info.swapchainCount = swapchain_count;
    present_info.pSwapchains = swapchain_handles.data();
    present_info.pImageIndices = swapchain_image_indices.data();
    present_info.pResults = results.data();

    VK_CHECK(vkQueuePresentKHR(mContext->present_queue.handle, &present_info), "Could not present");

    for (u32 i = 0; i < swapchain_count; i++) {
        swapchains[i]->waiting_to_present = false;
        if (results[i] == VK_ERROR_OUT_OF_DATE_KHR || results[i] == VK_SUBOPTIMAL_KHR) {
            swapchain_recreate(swapchain);
        }
    }
}

b8 VulkanBackend::submit_frame_command_buffers() {
    FrameData* current_frame = get_current_frame();

    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[] = { current_frame->present_semaphore };
    DynamicArray<VkSemaphore, BumpAllocator> wait_semaphores(mFrameAllocator, MAX_SWAPCHAIN_COUNT);

    // TODO: make this dynamic when more than 1 swapchain is supported
    for (u32 i = 0; i < MAX_SWAPCHAIN_COUNT; i++) {
        wait_semaphores[i] = mResources->swapchain.image_available_semaphores[mInFlightFrameIndex];
    }

    CommandBuffers& command_buffers = get_current_command_buffers();

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pWaitDstStageMask = wait_stages;
    // TODO: make this dynamic when more than 1 swapchain is supported
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores.data();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    submit_info.pCommandBuffers = command_buffers.array.data();
    submit_info.commandBufferCount = command_buffers.used_count;

    VK_CHECK(
        vkQueueSubmit(mContext->graphics_queue.handle, 1, &submit_info, current_frame->render_fence),
        "Could not submit for rendering");

    return true;
}

VkCommandBuffer VulkanBackend::get_command_buffer() {
    CommandBuffers& command_buffers = get_current_command_buffers();

    TK_ASSERT(command_buffers.used_count < MAX_IN_FLIGHT_COMMAND_BUFFER_COUNT, "Cannot get another command buffer");
    VkCommandBuffer cmd = command_buffers.array[command_buffers.used_count++];

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &command_buffer_begin_info), "Could not begin command buffer");

    return cmd;
}

RendererCommands* VulkanBackend::get_commands() {
    return new (mFrameAllocator->allocate(sizeof(VulkanCommands))) VulkanCommands(this);
}

void VulkanBackend::set_color_clear(const Vec4<f32>& c) {
    mSettings->color_clear = { { c.r, c.g, c.b, c.a } };
}

void VulkanBackend::set_depth_clear(f32 depth_clear) {
    mSettings->depth_stencil_clear.depth = depth_clear;
}

void VulkanBackend::set_stencil_clear(u32 stencil_clear) {
    mSettings->depth_stencil_clear.stencil = stencil_clear;
}

InternalShader* VulkanBackend::get_shader(const Handle handle) {
    ASSERT_SHADER(handle);
    return &mResources->shaders[handle];
}

void VulkanBackend::begin_rendering(VkCommandBuffer cmd, Handle framebuffer_handle, const Rect2D& a) {
    ASSERT_FRAMEBUFFER(framebuffer_handle);

    InternalFramebuffer& framebuffer = mResources->framebuffers[framebuffer_handle];
    u64 total_attachment_count = framebuffer.color_image->image_views.size();

    VkRenderingAttachmentInfo rendering_attachment_info{};
    rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    rendering_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    rendering_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    rendering_attachment_info.clearValue.color = mSettings->color_clear;

    DynamicArray<VkRenderingAttachmentInfo, BumpAllocator> rendering_attachment_infos(
        mFrameAllocator, total_attachment_count);

    for (u32 i = 0; i < total_attachment_count; i++) {
        rendering_attachment_infos[i] = rendering_attachment_info;
        rendering_attachment_infos[i].imageView = framebuffer.color_image->image_views[i];
    }

    VkCommandBuffer transition_layout_cmd = start_single_use_command_buffer();
    TransitionLayoutConfig transition_layout_config{};
    transition_layout_config.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    transition_layout_config.new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition_layout_config.src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    transition_layout_config.dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    transition_image_layouts(
        transition_layout_cmd, transition_layout_config, VK_IMAGE_ASPECT_COLOR_BIT, &framebuffer.color_image->image, 1);

    VkRenderingInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.layerCount = 1;
    rendering_info.renderArea = VkRect2D{ { a.pos.x, a.pos.y }, { a.size.x, a.size.y } };
    rendering_info.colorAttachmentCount = framebuffer.color_image->image_views.size();
    rendering_info.pColorAttachments = rendering_attachment_infos.data();

    if (framebuffer.depth_stencil_image) {
        VkRenderingAttachmentInfo* depth_stencil_attachment =
            mFrameAllocator->allocate<VkRenderingAttachmentInfo>(sizeof(VkRenderingAttachmentInfo));

        transition_layout_config.new_layout = framebuffer.has_stencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                                                                      : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        transition_layout_config.src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        transition_layout_config.dst_stage =
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        transition_image_layout(
            transition_layout_cmd, transition_layout_config, framebuffer.depth_stencil_image.data());

        *depth_stencil_attachment = rendering_attachment_info;
        depth_stencil_attachment->imageView = framebuffer.depth_stencil_image->image_views[0];
        depth_stencil_attachment->clearValue.depthStencil = mSettings->depth_stencil_clear;

        rendering_info.pDepthAttachment = depth_stencil_attachment;
        if (framebuffer.has_stencil) {
            rendering_info.pStencilAttachment = depth_stencil_attachment;
            depth_stencil_attachment->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } else {
            depth_stencil_attachment->imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        }
    }

    submit_single_use_command_buffer(transition_layout_cmd);

    vkCmdBeginRendering(cmd, &rendering_info);
}

void VulkanBackend::end_rendering(VkCommandBuffer cmd, Handle framebuffer_handle) {
    ASSERT_FRAMEBUFFER(framebuffer_handle);

    vkCmdEndRendering(cmd);

    InternalFramebuffer& framebuffer = mResources->framebuffers[framebuffer_handle];
    Swapchain& swapchain = mResources->swapchain;

    // TODO: make layer index be dynamic
    VkImageCopy image_copy{};
    image_copy.extent = framebuffer.color_image->extent;

    // Framebuffer color image subresoruce range
    image_copy.srcSubresource.baseArrayLayer = 0;
    image_copy.srcSubresource.mipLevel = 1;
    image_copy.srcSubresource.layerCount = 1;
    image_copy.srcSubresource.aspectMask = framebuffer.color_image->aspect_flags;

    // Swapchain image subresoruce range
    image_copy.dstSubresource.baseArrayLayer = 0;
    image_copy.dstSubresource.mipLevel = 1;
    image_copy.dstSubresource.layerCount = 1;
    image_copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    image_copy.dstOffset = {};
    image_copy.srcOffset = {};

    vkCmdCopyImage(
        cmd,
        framebuffer.color_image->image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        SWAPCHAIN_IMAGE(swapchain),
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        1,
        &image_copy);

    transition_swapchain_image(cmd, swapchain);
}

void VulkanBackend::bind_shader(VkCommandBuffer cmd, Shader const& shader) {
    ASSERT_SHADER(shader.handle);
    InternalPipeline& pipeline = mResources->shaders[shader.handle].pipelines[shader.handle.data];
    vkCmdBindPipeline(cmd, pipeline.bind_point, pipeline.pipeline);
}

void VulkanBackend::bind_buffer(VkCommandBuffer cmd, Buffer const& buffer) {
    ASSERT_BUFFER(buffer.handle);
    InternalBuffer& internal_buffer = mResources->buffers[buffer.handle];

    switch (internal_buffer.usage) {
        case VK_BUFFER_USAGE_VERTEX_BUFFER_BIT: {
            VkDeviceSize offsets[] = { 0 };
            VkBuffer buffers[] = { internal_buffer.buffer };
            vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
            break;
        }
        case VK_BUFFER_USAGE_INDEX_BUFFER_BIT: {
            VkDeviceSize offset = 0;
            VkBuffer index_buffer = internal_buffer.buffer;
            vkCmdBindIndexBuffer(cmd, index_buffer, offset, VK_INDEX_TYPE_UINT32);
            break;
        }
        default:
            TK_ASSERT(false, "Cannot bind buffer with this type");
    }
}

void VulkanBackend::draw(VkCommandBuffer cmd, u32 count) {
    vkCmdDraw(cmd, count, 1, 0, 0);
}

void VulkanBackend::draw_indexed(VkCommandBuffer cmd, u32 count) {
    draw_instanced(cmd, count, 1);
}

void VulkanBackend::draw_instanced(VkCommandBuffer cmd, u32 index_count, u32 instance_count) {
    vkCmdDrawIndexed(cmd, index_count, instance_count, 0, 0, 0);
}

void VulkanBackend::push_constants(
    VkCommandBuffer cmd,
    VkPipelineLayout layout,
    VkShaderStageFlags stage_flags,
    u32 offset,
    u32 size,
    const void* data) {
    vkCmdPushConstants(cmd, layout, stage_flags, offset, size, data);
}

void VulkanBackend::resources_initialize() {
    // Internal buffer creation
    {
        mResources->staging_buffer = buffer_internal_create(
            DEFAULT_STAGING_BUFFER_SIZE,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    // Command pool creation
    {
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.queueFamilyIndex = mContext->graphics_queue.family_index;

        mResources->command_pools = move(DynamicArray<VkCommandPool>(mAllocator, 1));

        VK_CHECK(
            vkCreateCommandPool(
                mContext->device,
                &command_pool_create_info,
                mContext->allocation_callbacks,
                mResources->command_pools.data()),
            "Could not create command pool(s)");

        mResources->extra_command_pools = move(DynamicArray<VkCommandPool>(mAllocator, 1));

        VK_CHECK(
            vkCreateCommandPool(
                mContext->device,
                &command_pool_create_info,
                mContext->allocation_callbacks,
                mResources->extra_command_pools.data()),
            "Could not create extra command pool(s)");
    }

    // Command buffer creation
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = mResources->command_pools[0];
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = MAX_IN_FLIGHT_COMMAND_BUFFER_COUNT;

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            mResources->command_buffers.array =
                move(StaticArray<VkCommandBuffer, MAX_IN_FLIGHT_COMMAND_BUFFER_COUNT>(mAllocator));
            mResources->command_buffers.used_count = 0;

            VK_CHECK(
                vkAllocateCommandBuffers(
                    mContext->device, &command_buffer_allocate_info, mResources->command_buffers.array.data()),
                "Could not allocate command buffers");
        }
    }

    // Frame data creation
    {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            VK_CHECK(
                vkCreateFence(
                    mContext->device, &fence_create_info, mContext->allocation_callbacks, &mFrames[i].render_fence),
                "Could not create render fence");
            VK_CHECK(
                vkCreateSemaphore(
                    mContext->device,
                    &semaphore_create_info,
                    mContext->allocation_callbacks,
                    &mFrames[i].present_semaphore),
                "Could not create present semaphore");
        }
    }
}

void VulkanBackend::resources_cleanup() {
    resources_wait();

    // Cleanup frames
    {
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroyFence(mContext->device, mFrames[i].render_fence, mContext->allocation_callbacks);
            vkDestroySemaphore(mContext->device, mFrames[i].present_semaphore, mContext->allocation_callbacks);
        }
    }

    // Cleanup command pools
    {
        for (u32 i = 0; i < mResources->command_pools.size(); i++) {
            vkDestroyCommandPool(mContext->device, mResources->command_pools[i], mContext->allocation_callbacks);
        }

        for (u32 i = 0; i < mResources->extra_command_pools.size(); i++) {
            vkDestroyCommandPool(mContext->device, mResources->extra_command_pools[i], mContext->allocation_callbacks);
        }
    }

    // Cleanup internal buffers
    buffer_internal_destroy(mResources->staging_buffer);
}

InternalBuffer VulkanBackend::buffer_internal_create(
    u32 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties) {
    InternalBuffer internal_buffer{};

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = size;
    buffer_create_info.usage = usage;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(
        vkCreateBuffer(mContext->device, &buffer_create_info, mContext->allocation_callbacks, &internal_buffer.buffer),
        "Could not create buffer");

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(mContext->device, internal_buffer.buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex =
        find_memory_type_index(memory_requirements.memoryTypeBits, memory_properties);

    VK_CHECK(
        vkAllocateMemory(
            mContext->device, &memory_allocate_info, mContext->allocation_callbacks, &internal_buffer.memory),
        "Could not allocate image memory");
    VK_CHECK(
        vkBindBufferMemory(mContext->device, internal_buffer.buffer, internal_buffer.memory, 0),
        "Could not bind buffer memory");

    internal_buffer.size = size;
    internal_buffer.usage = usage;
    internal_buffer.memory_requirements = memory_requirements;
    internal_buffer.memory_property_flags = memory_properties;

    return internal_buffer;
}

void VulkanBackend::buffer_internal_destroy(InternalBuffer& buffer) {
    vkFreeMemory(mContext->device, buffer.memory, mContext->allocation_callbacks);
    vkDestroyBuffer(mContext->device, buffer.buffer, mContext->allocation_callbacks);
    buffer = {};
}

InternalImage VulkanBackend::image_internal_create(
    u32 width,
    u32 height,
    u32 layer_count,
    VkFormat format,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_properties,
    VkImageAspectFlags aspect_flags) {
    InternalImage new_image(move(DynamicArray<VkImageView>(mAllocator, layer_count)));

    new_image.extent = VkExtent3D{ width, height, 1 };
    new_image.format = format;
    new_image.aspect_flags = aspect_flags;

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent = new_image.extent;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = layer_count;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = usage;

    VK_CHECK(
        vkCreateImage(mContext->device, &image_create_info, mContext->allocation_callbacks, &new_image.image),
        "Could not create image");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(mContext->device, new_image.image, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex =
        find_memory_type_index(memory_requirements.memoryTypeBits, memory_properties);

    VK_CHECK(
        vkAllocateMemory(mContext->device, &memory_allocate_info, mContext->allocation_callbacks, &new_image.memory),
        "Could not allocate image memory");
    VK_CHECK(vkBindImageMemory(mContext->device, new_image.image, new_image.memory, 0), "Could not bind image memory");

    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.image = new_image.image;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask = aspect_flags;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    for (u32 i = 0; i < layer_count; i++) {
        image_view_create_info.subresourceRange.baseArrayLayer = i;
        VK_CHECK(
            vkCreateImageView(
                mContext->device, &image_view_create_info, mContext->allocation_callbacks, &new_image.image_views[i]),
            "Could not create image view");
    }

    return new_image;
}

void VulkanBackend::image_internal_destroy(InternalImage& image) {
    vkFreeMemory(mContext->device, image.memory, mContext->allocation_callbacks);
    for (u32 i = 0; i < image.image_views.size(); i++) {
        vkDestroyImageView(mContext->device, image.image_views[i], mContext->allocation_callbacks);
    }
    vkDestroyImage(mContext->device, image.image, mContext->allocation_callbacks);
}

u32 VulkanBackend::find_memory_type_index(u32 type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(mContext->physical_device, &memory_properties);
    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    TK_ASSERT(false, "No correct memory type found");
    UNREACHABLE;
}

static VkFormat get_format(ColorFormat format_in) {
    switch (format_in) {
        case ColorFormat::RGBA8:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case ColorFormat::NONE:
            TK_ASSERT(false, "Color format not provided");
        default:
            TK_ASSERT(false, "TODO: add other formats");
    }

    UNREACHABLE;
};

static VkFormat get_depth_format(VkPhysicalDevice device, b8 has_stencil) {
    VkFormatFeatureFlags format_feature_flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(device, VK_FORMAT_D24_UNORM_S8_UINT, &format_properties);

    if ((format_properties.optimalTilingFeatures & format_feature_flags) == format_feature_flags) {
        return has_stencil ? VK_FORMAT_D24_UNORM_S8_UINT : VK_FORMAT_D32_SFLOAT;
    }

    TK_ASSERT(false, "GPU does not support depth/stencil format");
    UNREACHABLE;
}

VkImageMemoryBarrier VulkanBackend::create_image_memory_barrier(
    VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_flags) {
    VkImageMemoryBarrier image_memory_barrier{};
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.oldLayout = old_layout;
    image_memory_barrier.newLayout = new_layout;
    image_memory_barrier.srcQueueFamilyIndex = mContext->graphics_queue.family_index;
    image_memory_barrier.dstQueueFamilyIndex = mContext->graphics_queue.family_index;
    image_memory_barrier.subresourceRange.aspectMask = aspect_flags;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.levelCount = 1;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount = 1;

    switch (old_layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            image_memory_barrier.srcAccessMask = 0;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            image_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        default:
            TK_ASSERT(false, "Image layout transition not supported");
    }

    switch (new_layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            image_memory_barrier.dstAccessMask = 0;
            break;
        default:
            TK_ASSERT(false, "Image layout transition not supported");
    }

    return image_memory_barrier;
}

}  // namespace toki
