#include "vulkan_backend.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstring>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <utility>

#include "core/assert.h"
#include "core/base.h"
#include "core/logging.h"
#include "memory/allocators/basic_ref.h"
#include "renderer/vulkan/platform/vulkan_platform.h"
#include "renderer/vulkan/vulkan_types.h"
#include "resources/configs/shader_config_loader.h"
#include "resources/loaders/text_loader.h"
#include "vulkan/vulkan_core.h"

namespace toki {

namespace renderer {

#define ctx m_context

VulkanBackend::VulkanBackend(): m_context(&m_allocator) {
    ctx->internal_buffers = { MAX_ALLOCATED_BUFFER_COUNT };
    ctx->internal_images = { MAX_ALLOCATED_IMAGE_COUNT };
    ctx->internal_shaders = { MAX_SHADER_COUNT };
    ctx->internal_framebuffers = { MAX_RENDER_PASS_COUNT };
    create_instance();
}

VulkanBackend::~VulkanBackend() {
    cleanup_resources();
}

const Limits& VulkanBackend::limits() const {
    return ctx->limits;
}

const DeviceProperties& VulkanBackend::device_properties() const {
    return ctx->properties;
}

VkImage VulkanBackend::get_swapchain_image(u32 swapchain_index) {
    return ctx->swapchains[swapchain_index].images[ctx->swapchains[swapchain_index].image_index];
}

VkImageView VulkanBackend::get_swapchain_image_view(u32 swapchain_index) {
    return ctx->swapchains[swapchain_index].image_views[ctx->swapchains[swapchain_index].image_index];
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

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;

#ifndef TK_DIST
    {
        std::array validation_layers = {
            "VK_LAYER_KHRONOS_validation",
        };

        b8 layers_supported = true;

        uint32_t layer_count{};
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

        for (const char* required_layer : validation_layers) {
            b8 layer_found = false;

            for (const auto& found_layer : layers) {
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
        vkCreateInstance(&instance_create_info, ctx->allocation_callbacks, &ctx->instance),
        "Could not initialize renderer");
}

void VulkanBackend::find_physical_device(VkSurfaceKHR surface) {
    u32 physical_device_count{};
    vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, nullptr);
    TK_ASSERT(physical_device_count > 0, "No GPUs found");
    VkPhysicalDevice* physical_devices = m_frameAllocator->allocate_aligned<VkPhysicalDevice>(physical_device_count);
    vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, physical_devices);

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
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;

    for (u32 i = 0; i < physical_device_count; i++) {
        VkPhysicalDevice physical_device = physical_devices[i];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_device, &features);

        u32 score = rate_physical_device_suitability(physical_device, properties, features);
        if (score > best_score) {
            ctx->physical_device = physical_device;
            ctx->physical_device_properties = properties;
            device_properties = properties;
            device_features = features;
        }
    }

    ctx->limits.max_framebuffer_width = device_properties.limits.maxFramebufferWidth;
    ctx->limits.max_framebuffer_height = device_properties.limits.maxFramebufferHeight;
    ctx->limits.max_push_constant_size = device_properties.limits.maxPushConstantsSize;
    ctx->limits.max_color_attachments = device_properties.limits.maxColorAttachments;

    ctx->properties.depth_format = get_depth_format(ctx->physical_device, false);
    ctx->properties.depth_stencil_format = get_depth_format(ctx->physical_device, true);

    u32 queue_family_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical_device, &queue_family_count, nullptr);
    VkQueueFamilyProperties* queue_families =
        m_frameAllocator->allocate_aligned<VkQueueFamilyProperties>(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical_device, &queue_family_count, queue_families);

    for (u32 i = 0; i < queue_family_count; i++) {
        VkBool32 supports_present = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(ctx->physical_device, i, surface, &supports_present);
        if (supports_present && ctx->present_queue.family_index == -1) {
            ctx->present_queue.family_index = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && ctx->graphics_queue.family_index == -1) {
            ctx->graphics_queue.family_index = i;
        }
    }

    TK_ASSERT(ctx->present_queue.family_index != 1, "No queue family that supports presenting found");
    TK_ASSERT(ctx->graphics_queue.family_index != 1, "No queue family that supports graphics found");
}

void VulkanBackend::create_device(Window* window) {
    VkSurfaceKHR surface = renderer::create_surface(ctx->instance, ctx->allocation_callbacks, window);

    find_physical_device(surface);

    f32 queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_infos[2]{};
    queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[0].queueFamilyIndex = ctx->graphics_queue.family_index;
    queue_create_infos[0].queueCount = 1;
    queue_create_infos[0].pQueuePriorities = &queue_priority;
    queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[1].queueFamilyIndex = ctx->present_queue.family_index;
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
    device_create_info.enabledExtensionCount = sizeof(vulkan_extensions) / sizeof(vulkan_extensions[0]);
    device_create_info.ppEnabledExtensionNames = vulkan_extensions;
    device_create_info.pEnabledFeatures = &features;

    TK_LOG_INFO("Creating new Vulkan device");
    VK_CHECK(
        vkCreateDevice(ctx->physical_device, &device_create_info, ctx->allocation_callbacks, &ctx->device),
        "Could not create Vulkan device");

    vkGetDeviceQueue(ctx->device, ctx->graphics_queue.family_index, 0, &ctx->graphics_queue.handle);
    vkGetDeviceQueue(ctx->device, ctx->present_queue.family_index, 0, &ctx->present_queue.handle);

    vkDestroySurfaceKHR(ctx->instance, surface, ctx->allocation_callbacks);
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
                std::unreachable();
        }
    }

    return present_mode;
}

static VkExtent2D get_surface_extent(VkSurfaceCapabilitiesKHR* capabilities, Window* window) {
    if (capabilities->currentExtent.width != std::numeric_limits<u32>::max()) {
        return capabilities->currentExtent;
    }

    auto dimensions = window->get_dimensions();

    VkExtent2D extent{ static_cast<u32>(dimensions.x), static_cast<u32>(dimensions.y) };
    extent.width = std::clamp(extent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

    return extent;
}

void VulkanBackend::create_swapchain(Window* window) {
    TK_LOG_INFO("Creating swapchain");

    Swapchain swapchain{};
    swapchain.can_render = true;
    swapchain.window = window;
    VkSurfaceKHR surface = swapchain.surface = create_surface(ctx->instance, ctx->allocation_callbacks, window);

    // Query swapchain surface formats
    {
        u32 format_count{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, surface, &format_count, nullptr);
        TK_ASSERT(format_count > 0, "No surface formats found on physical device");

        VkSurfaceFormatKHR* surface_formats = m_frameAllocator->allocate_aligned<VkSurfaceFormatKHR>(format_count);

        vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, surface, &format_count, surface_formats);
        swapchain.surface_format = get_surface_format(surface_formats, format_count);
    }

    // Query non vsync fallback present mode
    {
        u32 present_mode_count{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical_device, surface, &present_mode_count, nullptr);
        TK_ASSERT(present_mode_count > 0, "No present modes found on physical device");

        VkPresentModeKHR* present_modes = m_frameAllocator->allocate_aligned<VkPresentModeKHR>(present_mode_count);

        vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physical_device, surface, &present_mode_count, present_modes);
        swapchain.vsync_disabled_present_mode = get_disabled_vsync_present_mode(present_modes, present_mode_count);
    }

    // Query swapchain image count
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physical_device, surface, &capabilities);
        swapchain.image_count =
            std::clamp(MAX_FRAMES_IN_FLIGHT, capabilities.minImageCount, capabilities.maxImageCount);
    }

    // Create mage available semaphore
    {
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VK_CHECK(
                vkCreateSemaphore(
                    ctx->device,
                    &semaphore_create_info,
                    ctx->allocation_callbacks,
                    &swapchain.image_available_semaphores[i]),
                "Could not create render semaphore");
        }
    }

    swapchain.image_views = BasicRef<VkImageView>(&m_allocator, swapchain.image_count);

    recreate_swapchain(&swapchain);

    window->get_event_handler().bind_event(EventType::WindowResize, this, [this, &swapchain](void*, void*, Event&) {
        if (!swapchain.can_render) {
            return;
        }

        this->wait_for_resources();
        TK_LOG_INFO("Recreating swapchain");
        this->recreate_swapchain(&swapchain);
    });

    window->get_event_handler().bind_event(EventType::WindowRestore, this, [&swapchain](void*, void*, Event&) {
        swapchain.can_render = true;
    });

    window->get_event_handler().bind_event(EventType::WindowMinimize, this, [&swapchain](void*, void*, Event&) {
        swapchain.can_render = false;
    });

    ctx->swapchains[ctx->swapchain_count++] = swapchain;
}

void VulkanBackend::recreate_swapchain(Swapchain* swapchain) {
    if (swapchain->image_views && swapchain->image_views.size() > 0) {
        VkImageView* image_views = swapchain->image_views.get();
        for (u32 i = 0; i < swapchain->image_count; i++) {
            vkDestroyImageView(ctx->device, image_views[i], ctx->allocation_callbacks);
        }
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physical_device, swapchain->surface, &capabilities);

    swapchain->extent = get_surface_extent(&capabilities, swapchain->window);
    TK_ASSERT(swapchain->extent.width > 0 && swapchain->extent.height > 0, "Surface extent is not of valid size");

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = swapchain->surface;
    swapchain_create_info.minImageCount = swapchain->image_count;
    swapchain_create_info.imageFormat = swapchain->surface_format.format;
    swapchain_create_info.imageColorSpace = swapchain->surface_format.colorSpace;
    swapchain_create_info.imageExtent = swapchain->extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode =
        ctx->vsync_enabled ? VK_PRESENT_MODE_FIFO_KHR : swapchain->vsync_disabled_present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = swapchain->swapchain;

    u32 queue_family_indices[] = { static_cast<u32>(ctx->present_queue.family_index),
                                   static_cast<u32>(ctx->graphics_queue.family_index) };

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
        vkCreateSwapchainKHR(ctx->device, &swapchain_create_info, ctx->allocation_callbacks, &swapchain->swapchain),
        "Could not create swapchain");

    if (swapchain_create_info.oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(ctx->device, swapchain_create_info.oldSwapchain, ctx->allocation_callbacks);
    }

    {
        vkGetSwapchainImagesKHR(ctx->device, swapchain->swapchain, &swapchain->image_count, nullptr);
        VkImage* swapchain_images = swapchain->images = BasicRef<VkImage>(&m_allocator, swapchain->image_count);
        vkGetSwapchainImagesKHR(ctx->device, swapchain->swapchain, &swapchain->image_count, swapchain_images);
        TK_ASSERT(swapchain->image_count > 0, "No images found for swapchain");

        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = swapchain->surface_format.format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        VkImageView* image_views = swapchain->image_views.get();

        for (u32 i = 0; i < swapchain->image_count; i++) {
            image_view_create_info.image = swapchain_images[i];
            VK_CHECK(
                vkCreateImageView(ctx->device, &image_view_create_info, ctx->allocation_callbacks, &image_views[i]),
                "Could not create image view");
        }
    }
}

Handle VulkanBackend::create_framebuffer(
    std::vector<ColorFormat> const& formats,
    Vec2 image_dimensions,
    b8 has_present_attachment,
    b8 has_depth,
    b8 has_stencil) {
    InternalFramebuffer framebuffer{};
    framebuffer.has_present_attachment = has_present_attachment;
    framebuffer.has_depth = has_depth;
    framebuffer.has_stencil = has_stencil;

    TK_ASSERT(
        (formats.size() + ((u8) has_present_attachment)) <= limits().max_color_attachments,
        "Maximum {} color attachments supported, including present attachment",
        limits().max_color_attachments);

    framebuffer.images = BasicRef<InternalImage>(&m_allocator, formats.size());
    framebuffer.formats = BasicRef<VkFormat>(&m_allocator, formats.size() + ((u8) has_present_attachment));

    if (has_present_attachment) {
        framebuffer.formats[0] = ctx->swapchains[0].surface_format.format;
    }

    VkExtent3D extent = { image_dimensions.x, image_dimensions.y, 1 };

    for (u32 i = 0; i < formats.size(); i++) {
        framebuffer.formats[i + (u8) has_present_attachment] = get_format(formats[i]);
        framebuffer.images[i] = create_image_internal(
            extent,
            framebuffer.formats[i],
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);
    }

    VkImageAspectFlags depthStenctilAspectFlags = 0;

    if (has_depth) {
        depthStenctilAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    if (has_stencil) {
        depthStenctilAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    if (depthStenctilAspectFlags) {
        framebuffer.depth_stencil_image = BasicRef<InternalImage>(&m_allocator);
        *framebuffer.depth_stencil_image.get() = create_image_internal(
            extent,
            get_depth_format(
                ctx->physical_device,
                (depthStenctilAspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT) == VK_IMAGE_ASPECT_STENCIL_BIT),
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            depthStenctilAspectFlags);
    }

    return ctx->internal_framebuffers.emplace(framebuffer);
}

void VulkanBackend::destroy_framebuffer(Handle framebuffer_handle) {
    TK_ASSERT(ctx->internal_framebuffers.contains(framebuffer_handle), "Framebuffer does not exist");

    InternalFramebuffer& framebuffer = ctx->internal_framebuffers[framebuffer_handle];
    for (u32 i = 0; i < framebuffer.images.size(); i++) {
        destroy_image_internal(&framebuffer.images[i]);
    }

    framebuffer.images.reset();
}

void VulkanBackend::destroy_swapchain(Handle& window_data_handle) {
    Swapchain& swapchain = ctx->swapchains[0];
    swapchain.window->get_event_handler().unbind_event(EventType::WindowResize, this);

    VkImageView* image_views = swapchain.image_views.get();

    for (u32 i = 0; i < swapchain.image_count; i++) {
        vkDestroyImageView(ctx->device, image_views[i], ctx->allocation_callbacks);
    }

    vkDestroySwapchainKHR(ctx->device, swapchain.swapchain, ctx->allocation_callbacks);
    vkDestroySurfaceKHR(ctx->instance, swapchain.surface, ctx->allocation_callbacks);

    window_data_handle.unique_id = INVALID_HANDLE_ID;
}

PipelineResources VulkanBackend::create_pipeline_resources(const std::vector<configs::Shader>& stages) {
    PipelineResources resources{};

    std::vector<VkPushConstantRange> push_constants;

    // for (const auto& stage : stages) {
    //     resources.binaries.emplace_back(stage.stage, create_shader_modules(stage));
    //     reflect_shader(stage.stage, resources.binaries.back().binary, resources.descriptor_bindings, push_constants);
    // }

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pSetLayouts = nullptr;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;

    VK_CHECK(
        vkCreatePipelineLayout(
            ctx->device, &pipeline_layout_create_info, ctx->allocation_callbacks, &resources.pipeline_layout),
        "Could not create pipeline layout");

    return resources;
}

Pipeline VulkanBackend::create_pipeline_internal(
    std::string_view config_path,
    VkFormat* attachment_formats,
    u64 attachment_count,
    VkFormat depth_format,
    VkFormat stencil_format) {
    auto config = configs::load_shader_config(config_path);

    Pipeline pipeline{};

    PipelineResources resources = create_pipeline_resources(config.stages);
    pipeline.pipeline_layout = resources.pipeline_layout;

    VkShaderModule* shader_modules = m_frameAllocator->allocate_aligned<VkShaderModule>(config.stages.size());
    VkPipelineShaderStageCreateInfo* shader_stage_create_infos =
        m_frameAllocator->allocate_aligned<VkPipelineShaderStageCreateInfo>(config.stages.size());
    for (u32 i = 0; i < config.stages.size(); i++) {
        ShaderModule module = create_shader_module(config.stages[i].stage, config.stages[i].path);
        shader_modules[i] = module.shader_module;
        shader_stage_create_infos[i] = module.shader_stage_create_info;
    }

    VkVertexInputBindingDescription* vertex_binding_descriptions =
        m_frameAllocator->allocate_aligned<VkVertexInputBindingDescription>(config.bindings.size());
    for (u32 i = 0; i < config.bindings.size(); i++) {
        vertex_binding_descriptions[i].binding = config.bindings[i].binding;
        vertex_binding_descriptions[i].stride = config.bindings[i].stride;

        switch (config.bindings[i].inputRate) {
            case VertexInputRate::Vertex:
                vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                break;
            case VertexInputRate::Instance:
                vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                break;
        }
    }

    VkVertexInputAttributeDescription* vertex_attribute_descriptions =
        m_frameAllocator->allocate_aligned<VkVertexInputAttributeDescription>(config.attributes.size());
    for (u32 i = 0; i < config.attributes.size(); i++) {
        vertex_attribute_descriptions[i].binding = config.attributes[i].binding;
        vertex_attribute_descriptions[i].offset = config.attributes[i].offset;
        vertex_attribute_descriptions[i].location = config.attributes[i].location;

        switch (config.attributes[i].format) {
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
                std::unreachable();
        }
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount = config.bindings.size();
    vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = config.attributes.size();
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

    switch (config.options.primitive_topology) {
        case PrimitiveTopology::PointList:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case PrimitiveTopology::LineList:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case PrimitiveTopology::LineStrip:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case PrimitiveTopology::TriangleList:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case PrimitiveTopology::TriangleStrip:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case PrimitiveTopology::TriangleFan:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            break;
        case PrimitiveTopology::LineListWithAdjacency:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::LineStripWithAdjacency:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::TriangleListWithAdjacency:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::TriangleStripWithAdjacency:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::PatchList:
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

    switch (config.options.front_face) {
        case FrontFace::Clockwise:
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
            break;
        case FrontFace::CounterClockwise:
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            break;
    }

    switch (config.options.polygon_mode) {
        case PolygonMode::Fill:
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            break;
        case PolygonMode::Line:
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_LINE;
            break;
        case PolygonMode::Point:
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_POINT;
            break;
    }

    switch (config.options.cull_mode) {
        case CullMode::None:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
            break;
        case CullMode::Front:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_BIT;
            break;
        case CullMode::Back:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case CullMode::Both:
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
    depth_stencil_state_create_info.depthTestEnable = config.options.depth_test_enable ? VK_TRUE : VK_FALSE;
    depth_stencil_state_create_info.depthWriteEnable = config.options.depth_write_enable ? VK_TRUE : VK_FALSE;
    depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_create_info.minDepthBounds = 0.0f;
    depth_stencil_state_create_info.maxDepthBounds = 1.0f;
    depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
    depth_stencil_state_create_info.front = {};
    depth_stencil_state_create_info.back = {};

    switch (config.options.depth_compare_op) {
        case CompareOp::Never:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_NEVER;
            break;
        case CompareOp::Less:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
            break;
        case CompareOp::Equal:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_EQUAL;
            break;
        case CompareOp::LessOrEqual:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            break;
        case CompareOp::Greater:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_GREATER;
            break;
        case CompareOp::NotEqual:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
            break;
        case CompareOp::GreaterOrEqual:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
            break;
        case CompareOp::Always:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_ALWAYS;
            break;
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

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states(
        attachment_count, color_blend_attachment_state);

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

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{};
    pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipeline_rendering_create_info.colorAttachmentCount = attachment_count;
    pipeline_rendering_create_info.pColorAttachmentFormats = attachment_formats;
    pipeline_rendering_create_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_create_info.stencilAttachmentFormat = stencil_format;

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.pNext = &pipeline_rendering_create_info;
    graphics_pipeline_create_info.stageCount = config.stages.size();
    graphics_pipeline_create_info.pStages = shader_stage_create_infos;
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
    graphics_pipeline_create_info.layout = resources.pipeline_layout;

    TK_LOG_INFO("Creating new graphics pipeline");
    VK_CHECK(
        vkCreateGraphicsPipelines(
            ctx->device,
            VK_NULL_HANDLE,
            1,
            &graphics_pipeline_create_info,
            ctx->allocation_callbacks,
            &pipeline.pipeline),
        "Could not create graphics pipeline");

    for (u32 i = 0; i < resources.binaries.size(); i++) {
        vkDestroyShaderModule(ctx->device, shader_modules[i], ctx->allocation_callbacks);
    }

    return pipeline;
}

void VulkanBackend::destroy_pipeline_internal(Pipeline* pipeline) {
    vkDestroyPipeline(ctx->device, pipeline->pipeline, ctx->allocation_callbacks);
    vkDestroyPipelineLayout(ctx->device, pipeline->pipeline_layout, ctx->allocation_callbacks);
}

Handle VulkanBackend::create_shader_internal(const Framebuffer* fb, const ShaderConfig& config) {
    TK_ASSERT(ctx->internal_framebuffers.contains(fb->handle), "No handle associated with provided handle");
    InternalFramebuffer& framebuffer = ctx->internal_framebuffers[fb->handle];

    InternalShader internal_shader{};
    internal_shader.pipelines[internal_shader.pipeline_count] = create_pipeline_internal(
        config.config_path,
        framebuffer.formats,
        framebuffer.formats.size(),
        framebuffer.has_depth ? ctx->properties.depth_format : VK_FORMAT_UNDEFINED);
    Handle handle = ctx->internal_shaders.emplace(internal_shader);
    handle.data = internal_shader.pipeline_count++;
    return handle;
}

void VulkanBackend::destroy_shader_internal(Handle shader_handle) {
    InternalShader& internal_shader = ctx->internal_shaders[shader_handle];
    for (u32 i = 0; i < internal_shader.pipeline_count; i++) {
        vkDestroyPipeline(ctx->device, internal_shader.pipelines[i].pipeline, ctx->allocation_callbacks);
        vkDestroyPipelineLayout(ctx->device, internal_shader.pipelines[i].pipeline_layout, ctx->allocation_callbacks);
    }

    for (u32 frame_index = 0; frame_index < MAX_FRAMES_IN_FLIGHT; frame_index++) {
        for (u32 set_index = 0; set_index < MAX_DESCRIPTOR_SET_COUNT; set_index++) {
            for (u32 binding_index = 0; binding_index < internal_shader.layout_counts[set_index]; binding_index++) {
                vkDestroyDescriptorSetLayout(
                    ctx->device,
                    internal_shader.descriptor_set_layouts[binding_index][set_index][frame_index],
                    ctx->allocation_callbacks);
            }
        }
    }
}

ShaderModule VulkanBackend::create_shader_module(ShaderStage stage, std::string_view path) {
    std::string shader_source = loaders::read_text_file(path);

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);

#ifdef TK_DIST
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

    shaderc_shader_kind shader_kind;

    switch (stage) {
        case ShaderStage::Vertex:
            shader_kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
            break;
        case ShaderStage::Fragment:
            shader_kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
            break;
        default:
            TK_ASSERT(false, "No other shader stage supported");
    }

    shaderc::SpvCompilationResult spirv_module =
        compiler.CompileGlslToSpv(shader_source.data(), shader_kind, ".", options);
    if (spirv_module.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
        TK_LOG_ERROR(
            "ERROR MESSAGE:\n{}\n\nCOMPILATION STATUS: {}",
            spirv_module.GetErrorMessage(),
            (int) spirv_module.GetCompilationStatus());
    }
    TK_ASSERT(
        spirv_module.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success,
        "Shader compilation error");

    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.pCode = reinterpret_cast<const u32*>(spirv_module.begin());
    shader_module_create_info.codeSize = ((uintptr_t) spirv_module.end()) - ((uintptr_t) spirv_module.begin());

    ShaderModule shader_module{};
    VK_CHECK(
        vkCreateShaderModule(
            ctx->device, &shader_module_create_info, ctx->allocation_callbacks, &shader_module.shader_module),
        "Could not create shader module");

    VkPipelineShaderStageCreateInfo& shader_stage_create_info = shader_module.shader_stage_create_info;
    shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_create_info.module = shader_module.shader_module;
    shader_stage_create_info.pName = "main";

    switch (stage) {
        case ShaderStage::Vertex:
            shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case ShaderStage::Fragment:
            shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        default:
            std::unreachable();
    }

    return shader_module;
}

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
            std::unreachable();
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
}

void VulkanBackend::prepare_swapchain_frame(Swapchain* swapchain) {
    VkResult result = vkAcquireNextImageKHR(
        ctx->device,
        swapchain->swapchain,
        UINT64_MAX,
        swapchain->image_available_semaphores[m_inFlightFrameIndex],
        VK_NULL_HANDLE,
        &swapchain->image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain(swapchain);
        return;
    } else {
        TK_ASSERT(
            result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR || result == VK_NOT_READY,
            "Could not acquire swapchain image");
    }
}

void VulkanBackend::reset_swapchain_frame(Swapchain* swapchain) {
    swapchain->is_recording_commands = false;
}

VkCommandBuffer VulkanBackend::start_single_use_command_buffer() {
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandPool = ctx->extra_command_pools[0];
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(ctx->device, &command_buffer_allocate_info, &command_buffer);

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

    vkQueueSubmit(ctx->graphics_queue.handle, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(ctx->graphics_queue.handle);
}

FrameData* VulkanBackend::get_current_frame() {
    return &m_frames.data[m_inFlightFrameIndex];
}

CommandBuffers* VulkanBackend::get_current_command_buffers() {
    return &ctx->command_buffers[m_inFlightFrameIndex];
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

    VkImageMemoryBarrier* image_memory_barriers = m_frameAllocator->allocate_aligned<VkImageMemoryBarrier>(image_count);
    for (u32 i = 0; i < image_count; i++) {
        image_memory_barriers[i] = image_memory_barrier;
        image_memory_barriers[i].image = images[i];
    }

    vkCmdPipelineBarrier(
        cmd, config.src_stage, config.dst_stage, 0, 0, nullptr, 0, nullptr, image_count, image_memory_barriers);
}

void VulkanBackend::wait_for_resources() {
    vkDeviceWaitIdle(ctx->device);
}

void VulkanBackend::prepare_frame_resources() {
    FrameData* frame = get_current_frame();

    VkResult result = vkWaitForFences(ctx->device, 1, &frame->render_fence, VK_TRUE, UINT64_MAX);
    TK_ASSERT(result == VK_SUCCESS || result == VK_TIMEOUT, "Failed waiting for fences");

    for (u32 i = 0; i < 1; i++) {
        prepare_swapchain_frame(&ctx->swapchains[i]);
    }

    VK_CHECK(vkResetFences(ctx->device, 1, &frame->render_fence), "Could not reset render fence");

    {
        CommandBuffers* command_buffers = get_current_command_buffers();
        for (u32 i = 0; i < command_buffers->count; i++) {
            vkResetCommandBuffer(command_buffers->command_buffers[i], 0);
        }
        command_buffers->count = 0;
    }
}

void VulkanBackend::submit_commands() {
    CommandBuffers* command_buffers = get_current_command_buffers();
    for (u32 i = 0; i < command_buffers->count; i++) {
        vkEndCommandBuffer(command_buffers->command_buffers[i]);
    }

    submit_frame_command_buffers();
}

void VulkanBackend::cleanup_frame_resources() {
    for (auto& swapchain : ctx->swapchains) {
        reset_swapchain_frame(&swapchain);
    }

    m_inFlightFrameIndex = (m_inFlightFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    m_frameAllocator.swap();
    m_frameAllocator->clear();

    ctx->staging_buffer_offset = 0;
}

void VulkanBackend::present() {
    FrameData* frame = get_current_frame();
    VkSemaphore wait_semaphores[] = { frame->present_semaphore };

    u32 swapchain_count = 1;
    VkSwapchainKHR* swapchain_handles = m_frameAllocator->allocate_aligned<VkSwapchainKHR>(swapchain_count);
    u32* swapchain_image_indices = m_frameAllocator->allocate_aligned<u32>(swapchain_count);
    Swapchain** swapchains = m_frameAllocator->allocate_aligned<Swapchain*>(swapchain_count);

    swapchain_count = 0;

    for (auto& swapchain : ctx->swapchains) {
        if (!swapchain.can_render) {
            continue;
        }

        swapchain_handles[swapchain_count] = swapchain.swapchain;
        swapchain_image_indices[swapchain_count] = swapchain.image_index;
        swapchains[swapchain_count] = &swapchain;
        swapchain_count++;
    }

    VkResult* results = m_frameAllocator->allocate_aligned<VkResult>(swapchain_count);

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = wait_semaphores;
    present_info.swapchainCount = swapchain_count;
    present_info.pSwapchains = swapchain_handles;
    present_info.pImageIndices = swapchain_image_indices;
    present_info.pResults = results;

    VK_CHECK(vkQueuePresentKHR(ctx->present_queue.handle, &present_info), "Could not present");

    for (u32 i = 0; i < swapchain_count; i++) {
        swapchains[i]->waiting_to_present = false;
        if (results[i] == VK_ERROR_OUT_OF_DATE_KHR || results[i] == VK_SUBOPTIMAL_KHR) {
            recreate_swapchain(swapchains[i]);
        }
    }
}

b8 VulkanBackend::submit_frame_command_buffers() {
    FrameData* current_frame = get_current_frame();

    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[] = { current_frame->present_semaphore };
    VkSemaphore* wait_semaphores = m_frameAllocator->allocate_aligned<VkSemaphore>(MAX_SWAPCHAIN_COUNT);

    // TODO: make this dynamic when more than 1 swapchain is supported
    for (u32 i = 0; i < MAX_SWAPCHAIN_COUNT; i++) {
        wait_semaphores[i] = ctx->swapchains[i].image_available_semaphores[m_inFlightFrameIndex];
    }

    CommandBuffers* command_buffers = get_current_command_buffers();

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pWaitDstStageMask = wait_stages;
    // TODO: make this dynamic when more than 1 swapchain is supported
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    submit_info.pCommandBuffers = command_buffers->command_buffers;
    submit_info.commandBufferCount = command_buffers->count;

    VK_CHECK(
        vkQueueSubmit(ctx->graphics_queue.handle, 1, &submit_info, current_frame->render_fence),
        "Could not submit for rendering");

    return true;
}

VkCommandBuffer VulkanBackend::get_command_buffer() {
    CommandBuffers* command_buffers = get_current_command_buffers();

    VkCommandBuffer cmd = command_buffers->command_buffers[command_buffers->count++];

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &command_buffer_begin_info), "Could not begin command buffer");

    return cmd;
}

RendererCommands* VulkanBackend::get_commands() {
    return m_frameAllocator->emplace<VulkanCommands>(this);
}

void VulkanBackend::set_color_clear(const glm::vec4& c) {
    ctx->color_clear = { { c.r, c.g, c.b, c.a } };
}

void VulkanBackend::set_depth_clear(f32 depth_clear) {
    ctx->depth_stencil_clear.depth = depth_clear;
}

void VulkanBackend::set_stencil_clear(u32 stencil_clear) {
    ctx->depth_stencil_clear.stencil = stencil_clear;
}

void VulkanBackend::begin_rendering(VkCommandBuffer cmd, Handle framebuffer_handle, const Rect2D& a) {
    TK_ASSERT(
        ctx->internal_framebuffers.contains(framebuffer_handle), "No framebuffer associated with provided handle");

    InternalFramebuffer& framebuffer = ctx->internal_framebuffers[framebuffer_handle];
    u64 total_attachment_count = framebuffer.images.size() + ((u8) framebuffer.has_present_attachment);

    VkRenderingAttachmentInfo rendering_attachment_info{};
    rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    rendering_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    rendering_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    rendering_attachment_info.clearValue.color = ctx->color_clear;

    VkRenderingAttachmentInfo* rendering_attachment_infos =
        m_frameAllocator->allocate_aligned<VkRenderingAttachmentInfo>(total_attachment_count);
    VkImage* images = m_frameAllocator->allocate_aligned<VkImage>(total_attachment_count);

    u32 start_index = 0;
    if (framebuffer.has_present_attachment) {
        images[0] = get_swapchain_image(framebuffer.swapchain_index);
        rendering_attachment_infos[0] = rendering_attachment_info;
        rendering_attachment_infos[0].imageView = get_swapchain_image_view(framebuffer.swapchain_index);
        start_index = 1;
    }

    for (u32 i = start_index; i < total_attachment_count; i++) {
        u32 index = i - start_index;
        rendering_attachment_infos[index] = rendering_attachment_info;
        rendering_attachment_infos[index].imageView = framebuffer.images[index].image_view;
    }

    VkCommandBuffer transition_layout_cmd = start_single_use_command_buffer();
    TransitionLayoutConfig transition_layout_config{};
    transition_layout_config.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    transition_layout_config.new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    transition_layout_config.src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    transition_layout_config.dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    transition_image_layouts(
        transition_layout_cmd, transition_layout_config, VK_IMAGE_ASPECT_COLOR_BIT, images, total_attachment_count);

    VkRenderingInfo rendering_info{};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.layerCount = 1;
    rendering_info.renderArea = VkRect2D{ { a.pos.x, a.pos.y }, { a.size.x, a.size.y } };
    rendering_info.colorAttachmentCount = framebuffer.formats.size();
    rendering_info.pColorAttachments = rendering_attachment_infos;

    if (framebuffer.depth_stencil_image) {
        VkRenderingAttachmentInfo* depth_stencil_attachment =
            m_frameAllocator->allocate_aligned<VkRenderingAttachmentInfo>(1);

        transition_layout_config.new_layout = framebuffer.has_stencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                                                                      : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        transition_layout_config.src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        transition_layout_config.dst_stage =
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        transition_image_layout(transition_layout_cmd, transition_layout_config, framebuffer.depth_stencil_image.get());

        *depth_stencil_attachment = rendering_attachment_info;
        depth_stencil_attachment->imageView = framebuffer.depth_stencil_image->image_view;
        depth_stencil_attachment->clearValue = {};
        depth_stencil_attachment->clearValue.depthStencil = { 1.0f, 0 };

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
    TK_ASSERT(
        ctx->internal_framebuffers.contains(framebuffer_handle), "No framebuffer associated with provided handle");

    vkCmdEndRendering(cmd);

    InternalFramebuffer& framebuffer = ctx->internal_framebuffers[framebuffer_handle];
    if (framebuffer.has_present_attachment) {
        TransitionLayoutConfig transition_layout_config{};
        transition_layout_config.old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        transition_layout_config.new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        transition_layout_config.src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        transition_layout_config.dst_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        VkCommandBuffer transition_layout_cmd = start_single_use_command_buffer();
        transition_image_layout(
            transition_layout_cmd,
            transition_layout_config,
            VK_IMAGE_ASPECT_COLOR_BIT,
            get_swapchain_image(framebuffer.swapchain_index));
        submit_single_use_command_buffer(transition_layout_cmd);
    }
}

void VulkanBackend::bind_shader(VkCommandBuffer cmd, Shader const& shader) {
    Pipeline& pipeline = ctx->internal_shaders[shader.handle].pipelines[shader.handle.data];
    vkCmdBindPipeline(cmd, pipeline.bind_point, pipeline.pipeline);
}

void VulkanBackend::bind_buffer(VkCommandBuffer cmd, Buffer const& buffer) {
    TK_ASSERT(ctx->internal_buffers.contains(buffer.handle), "Buffer with provided handle does not exist");
    InternalBuffer& internal_buffer = ctx->internal_buffers[buffer.handle];

    switch (buffer.type) {
        case BufferType::Vertex: {
            VkDeviceSize offsets[] = { 0 };
            VkBuffer buffers[] = { internal_buffer.buffer };
            vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
            break;
        }
        case BufferType::Index: {
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

void VulkanBackend::initialize_resources() {
    // Internal buffer creation
    {
        ctx->staging_buffer = {};
        InternalBuffer buffer = create_buffer_internal(
            DEFAULT_STAGING_BUFFER_SIZE,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        ctx->staging_buffer = buffer;
    }

    // Command pool creation
    {
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.queueFamilyIndex = ctx->graphics_queue.family_index;

        ctx->command_pools = BasicRef<VkCommandPool>(&m_allocator, 1);

        VK_CHECK(
            vkCreateCommandPool(
                ctx->device, &command_pool_create_info, ctx->allocation_callbacks, ctx->command_pools.get()),
            "Could not create command pool(s)");

        ctx->extra_command_pools = BasicRef<VkCommandPool>(&m_allocator, 1);

        VK_CHECK(
            vkCreateCommandPool(
                ctx->device, &command_pool_create_info, ctx->allocation_callbacks, ctx->extra_command_pools.get()),
            "Could not create extra command pool(s)");
    }

    // Command buffer creation
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = ctx->command_pools[0];
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = MAX_IN_FLIGHT_COMMAND_BUFFERS;

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            ctx->command_buffers[i].command_buffers = { &m_allocator, MAX_IN_FLIGHT_COMMAND_BUFFERS };
            ctx->command_buffers[i].count = 0;

            VK_CHECK(
                vkAllocateCommandBuffers(
                    ctx->device, &command_buffer_allocate_info, ctx->command_buffers[i].command_buffers),
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
                    ctx->device, &fence_create_info, ctx->allocation_callbacks, &m_frames.data[i].render_fence),
                "Could not create render fence");
            VK_CHECK(
                vkCreateSemaphore(
                    ctx->device,
                    &semaphore_create_info,
                    ctx->allocation_callbacks,
                    &m_frames.data[i].present_semaphore),
                "Could not create present semaphore");
        }
    }
}

void VulkanBackend::cleanup_resources() {
    wait_for_resources();

    // Cleanup frames
    {
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroyFence(ctx->device, m_frames.data[i].render_fence, ctx->allocation_callbacks);
            vkDestroySemaphore(ctx->device, m_frames.data[i].present_semaphore, ctx->allocation_callbacks);
        }
    }

    // Cleanup command pools
    {
        for (u32 i = 0; i < ctx->command_pools.size(); i++) {
            vkDestroyCommandPool(ctx->device, ctx->command_pools[i], ctx->allocation_callbacks);
        }

        for (u32 i = 0; i < ctx->extra_command_pools.size(); i++) {
            vkDestroyCommandPool(ctx->device, ctx->extra_command_pools[i], ctx->allocation_callbacks);
        }
    }

    // Cleanup internal buffers
    destroy_buffer_internal(&ctx->staging_buffer);
}

Handle VulkanBackend::create_buffer(BufferType type, u32 size) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    switch (type) {
        case BufferType::Vertex:
            usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case BufferType::Index:
            usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        default:
            std::unreachable();
    }

    return ctx->internal_buffers.emplace(create_buffer_internal(size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
}

void VulkanBackend::destroy_buffer(Buffer* buffer) {
    TK_ASSERT(buffer && buffer->size, "Invalid buffer provided");
    TK_ASSERT(ctx->internal_buffers.contains(buffer->handle), "Buffer with provided handle does not exist");

    InternalBuffer& internal_buffer = ctx->internal_buffers[buffer->handle];
    destroy_buffer_internal(&internal_buffer);

    *buffer = {};
}

void* VulkanBackend::map_buffer_memory(VkDeviceMemory memory, u32 offset, u32 size) {
    void* data{};
    VK_CHECK(vkMapMemory(ctx->device, memory, offset, size, 0, &data), "Could not map buffer memory");
    return data;
}

void VulkanBackend::unmap_buffer_memory(VkDeviceMemory memory) {
    vkUnmapMemory(ctx->device, memory);
}

void VulkanBackend::flush_buffer(Buffer* buffer) {
    TK_ASSERT(ctx->internal_buffers.contains(buffer->handle), "Buffer with provided handle does not exist");
    InternalBuffer& internal_buffer = ctx->internal_buffers[buffer->handle];

    if ((internal_buffer.memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
        VkMappedMemoryRange mapped_memory_range{};
        mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mapped_memory_range.memory = internal_buffer.memory;
        mapped_memory_range.offset = 0;
        mapped_memory_range.size = buffer->size;
        VK_CHECK(vkFlushMappedMemoryRanges(ctx->device, 1, &mapped_memory_range), "Could not flush buffer memory");
    }
}

void VulkanBackend::set_buffer_data(Buffer* buffer, u32 size, void* data) {
    TK_ASSERT(ctx->internal_buffers.contains(buffer->handle), "Buffer with provided handle does not exist");
    InternalBuffer& staging_buffer = ctx->staging_buffer;

    // Copy to staging buffer
    void* mapped_data = map_buffer_memory(staging_buffer.memory, ctx->staging_buffer_offset, size);
    std::memcpy(mapped_data, data, size);
    unmap_buffer_memory(staging_buffer.memory);
    mapped_data = nullptr;

    InternalBuffer& internal_buffer = ctx->internal_buffers[buffer->handle];

    // Copy to uploaded buffer
    copy_buffer_data(internal_buffer.buffer, staging_buffer.buffer, size, 0, ctx->staging_buffer_offset);

    ctx->staging_buffer_offset += size;
}

void VulkanBackend::copy_buffer_data(VkBuffer dst, VkBuffer src, u32 size, u32 dst_offset, u32 src_offset) {
    VkCommandBuffer cmd = start_single_use_command_buffer();

    VkBufferCopy buffer_copy{};
    buffer_copy.srcOffset = src_offset;
    buffer_copy.dstOffset = dst_offset;
    buffer_copy.size = size;
    vkCmdCopyBuffer(cmd, src, dst, 1, &buffer_copy);

    submit_single_use_command_buffer(cmd);
}

Handle VulkanBackend::create_image(ColorFormat format, u32 width, u32 height) {
    VkExtent3D extent = { width, height, 1 };

    InternalImage new_image = create_image_internal(
        extent,
        get_format(format),
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT);

    return ctx->internal_images.emplace(new_image);
}

void VulkanBackend::destroy_image(Handle image_handle) {
    TK_ASSERT(ctx->internal_images.contains(image_handle), "Handle is not associated with any Vulkan image");
    ctx->internal_images.erase(image_handle);
}

InternalBuffer VulkanBackend::create_buffer_internal(
    u32 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties) {
    InternalBuffer buffer{};

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = size;
    buffer_create_info.usage = usage;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(
        vkCreateBuffer(ctx->device, &buffer_create_info, ctx->allocation_callbacks, &buffer.buffer),
        "Could not create buffer");

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(ctx->device, buffer.buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex =
        find_memory_type_index(memory_requirements.memoryTypeBits, memory_properties);

    VK_CHECK(
        vkAllocateMemory(ctx->device, &memory_allocate_info, ctx->allocation_callbacks, &buffer.memory),
        "Could not allocate image memory");
    VK_CHECK(vkBindBufferMemory(ctx->device, buffer.buffer, buffer.memory, 0), "Could not bind buffer memory");

    buffer.usage = usage;
    buffer.memory_requirements = memory_requirements;
    buffer.memory_property_flags = memory_properties;

    return buffer;
}

void VulkanBackend::destroy_buffer_internal(InternalBuffer* buffer) {
    vkFreeMemory(ctx->device, buffer->memory, ctx->allocation_callbacks);
    vkDestroyBuffer(ctx->device, buffer->buffer, ctx->allocation_callbacks);
    *buffer = {};
}

InternalImage VulkanBackend::create_image_internal(
    VkExtent3D extent,
    VkFormat format,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_properties,
    VkImageAspectFlags aspect_flags) {
    InternalImage image{};
    image.format = format;
    image.aspect_flags = aspect_flags;

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent = extent;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = usage;

    VK_CHECK(
        vkCreateImage(ctx->device, &image_create_info, ctx->allocation_callbacks, &image.image),
        "Could not create image");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(ctx->device, image.image, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex =
        find_memory_type_index(memory_requirements.memoryTypeBits, memory_properties);

    VK_CHECK(
        vkAllocateMemory(ctx->device, &memory_allocate_info, ctx->allocation_callbacks, &image.memory),
        "Could not allocate image memory");
    VK_CHECK(vkBindImageMemory(ctx->device, image.image, image.memory, 0), "Could not bind image memory");

    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.image = image.image;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask = aspect_flags;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(
        vkCreateImageView(ctx->device, &image_view_create_info, ctx->allocation_callbacks, &image.image_view),
        "Could not create image view");

    return image;
}

void VulkanBackend::destroy_image_internal(InternalImage* image) {
    vkFreeMemory(ctx->device, image->memory, ctx->allocation_callbacks);
    vkDestroyImageView(ctx->device, image->image_view, ctx->allocation_callbacks);
    vkDestroyImage(ctx->device, image->image, ctx->allocation_callbacks);
    *image = {};
}

u32 VulkanBackend::find_memory_type_index(u32 type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(ctx->physical_device, &memory_properties);
    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    TK_ASSERT(false, "No correct memory type found");
    std::unreachable();
}

static VkFormat get_format(ColorFormat format_in) {
    switch (format_in) {
        case ColorFormat::RGBA8:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case ColorFormat::None:
            TK_ASSERT(false, "Color format not provided");
        default:
            TK_ASSERT(false, "TODO: add other formats");
    }
};

static VkFormat get_depth_format(VkPhysicalDevice device, b8 has_stencil) {
    VkFormatFeatureFlags format_feature_flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(device, VK_FORMAT_D24_UNORM_S8_UINT, &format_properties);

    if ((format_properties.optimalTilingFeatures & format_feature_flags) == format_feature_flags) {
        return has_stencil ? VK_FORMAT_D24_UNORM_S8_UINT : VK_FORMAT_D32_SFLOAT;
    }

    TK_ASSERT(false, "GPU does not support depth/stencil format");
    std::unreachable();
}

VkImageMemoryBarrier VulkanBackend::create_image_memory_barrier(
    VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_flags) {
    VkImageMemoryBarrier image_memory_barrier{};
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.oldLayout = old_layout;
    image_memory_barrier.newLayout = new_layout;
    image_memory_barrier.srcQueueFamilyIndex = ctx->graphics_queue.family_index;
    image_memory_barrier.dstQueueFamilyIndex = ctx->graphics_queue.family_index;
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

}  // namespace renderer

}  // namespace toki
