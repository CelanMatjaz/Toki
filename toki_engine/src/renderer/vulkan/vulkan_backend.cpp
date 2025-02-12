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
#include "renderer/vulkan/platform/vulkan_platform.h"
#include "renderer/vulkan/vulkan_types.h"
#include "resources/loaders/text_loader.h"
#include "vulkan/vulkan_core.h"

namespace toki {

namespace vulkan_renderer {

VulkanBackend::VulkanBackend() {
    m_context.render_passes = { MAX_RENDER_PASS_COUNT };
    m_context.pipelines = { MAX_PIPELINE_COUNT };
    m_context.buffers = { MAX_BUFFER_COUNT };
    create_instance();
}

VulkanBackend::~VulkanBackend() {
    cleanup_resources();
}

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
        vkCreateInstance(&instance_create_info, m_context.allocation_callbacks, &m_context.instance),
        "Could not initialize renderer");
}

void VulkanBackend::find_physical_device(VkSurfaceKHR surface) {
    u32 physical_device_count{};
    vkEnumeratePhysicalDevices(m_context.instance, &physical_device_count, nullptr);
    TK_ASSERT(physical_device_count > 0, "No GPUs found");
    VkPhysicalDevice* physical_devices = m_frameAllocator->allocate_aligned<VkPhysicalDevice>(physical_device_count);
    vkEnumeratePhysicalDevices(m_context.instance, &physical_device_count, physical_devices);

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
    for (u32 i = 0; i < physical_device_count; i++) {
        VkPhysicalDevice physical_device = physical_devices[i];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_device, &features);

        u32 score = rate_physical_device_suitability(physical_device, properties, features);
        if (score > best_score) {
            m_context.physical_device = physical_device;
            m_context.physical_device_properties = properties;
        }
    }

    u32 queue_family_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(m_context.physical_device, &queue_family_count, nullptr);
    VkQueueFamilyProperties* queue_families =
        m_frameAllocator->allocate_aligned<VkQueueFamilyProperties>(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_context.physical_device, &queue_family_count, queue_families);

    for (u32 i = 0; i < queue_family_count; i++) {
        VkBool32 supports_present = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_context.physical_device, i, surface, &supports_present);
        if (supports_present && m_context.present_queue.family_index == -1) {
            m_context.present_queue.family_index = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && m_context.graphics_queue.family_index == -1) {
            m_context.graphics_queue.family_index = i;
        }
    }

    TK_ASSERT(m_context.present_queue.family_index != 1, "No queue family that supports presenting found");
    TK_ASSERT(m_context.graphics_queue.family_index != 1, "No queue family that supports graphics found");
}

void VulkanBackend::create_device(Window* window) {
    VkSurfaceKHR surface = create_surface(m_context.instance, m_context.allocation_callbacks, window);

    find_physical_device(surface);

    f32 queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_infos[2]{};
    queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[0].queueFamilyIndex = m_context.graphics_queue.family_index;
    queue_create_infos[0].queueCount = 1;
    queue_create_infos[0].pQueuePriorities = &queue_priority;
    queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[1].queueFamilyIndex = m_context.present_queue.family_index;
    queue_create_infos[1].queueCount = 1;
    queue_create_infos[1].pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures features{};
    features.fillModeNonSolid = VK_TRUE;
    features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.enabledExtensionCount = sizeof(vulkan_extensions) / sizeof(vulkan_extensions[0]);
    device_create_info.ppEnabledExtensionNames = vulkan_extensions;
    device_create_info.pEnabledFeatures = &features;

    TK_LOG_INFO("Creating new Vulkan device");
    VK_CHECK(
        vkCreateDevice(
            m_context.physical_device, &device_create_info, m_context.allocation_callbacks, &m_context.device),
        "Could not create Vulkan device");

    vkGetDeviceQueue(m_context.device, m_context.graphics_queue.family_index, 0, &m_context.graphics_queue.handle);
    vkGetDeviceQueue(m_context.device, m_context.present_queue.family_index, 0, &m_context.present_queue.handle);

    vkDestroySurfaceKHR(m_context.instance, surface, m_context.allocation_callbacks);
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

static VkExtent2D get_surface_extent(VkSurfaceCapabilitiesKHR* capabilities, Swapchain* swapchain) {
    if (capabilities->currentExtent.width != std::numeric_limits<u32>::max()) {
        return capabilities->currentExtent;
    }

    auto dimensions = swapchain->window->get_dimensions();

    VkExtent2D extent{ static_cast<u32>(dimensions.x), static_cast<u32>(dimensions.y) };
    extent.width = std::clamp(extent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

    return extent;
}

Handle VulkanBackend::create_swapchain(Window* window) {
    TK_LOG_INFO("Creating swapchain");

    Swapchain swapchain{};
    swapchain.window = window;
    swapchain.can_render = true;
    VkSurfaceKHR surface = swapchain.surface =
        create_surface(m_context.instance, m_context.allocation_callbacks, window);

    // Query swapchain surface formats
    {
        u32 format_count{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_context.physical_device, surface, &format_count, nullptr);
        TK_ASSERT(format_count > 0, "No surface formats found on physical device");

        VkSurfaceFormatKHR* surface_formats = m_frameAllocator->allocate_aligned<VkSurfaceFormatKHR>(format_count);

        vkGetPhysicalDeviceSurfaceFormatsKHR(m_context.physical_device, surface, &format_count, surface_formats);
        swapchain.surface_format = get_surface_format(surface_formats, format_count);
    }

    // Query non vsync fallback present mode
    {
        u32 present_mode_count{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_context.physical_device, surface, &present_mode_count, nullptr);
        TK_ASSERT(present_mode_count > 0, "No present modes found on physical device");

        VkPresentModeKHR* present_modes = m_frameAllocator->allocate_aligned<VkPresentModeKHR>(present_mode_count);

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            m_context.physical_device, surface, &present_mode_count, present_modes);
        swapchain.vsync_disabled_present_mode = get_disabled_vsync_present_mode(present_modes, present_mode_count);
    }

    // Query swapchain image count
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context.physical_device, surface, &capabilities);
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
                    m_context.device,
                    &semaphore_create_info,
                    m_context.allocation_callbacks,
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

    m_context.swapchains[0] = swapchain;

    return {};
}

void VulkanBackend::recreate_swapchain(Swapchain* swapchain) {
    if (swapchain->image_views && swapchain->image_views.size() > 0) {
        VkImageView* image_views = swapchain->image_views.get();
        for (u32 i = 0; i < swapchain->image_count; i++) {
            vkDestroyImageView(m_context.device, image_views[i], m_context.allocation_callbacks);
        }
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context.physical_device, swapchain->surface, &capabilities);

    swapchain->extent = get_surface_extent(&capabilities, swapchain);
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
        m_context.vsync_enabled ? VK_PRESENT_MODE_FIFO_KHR : swapchain->vsync_disabled_present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = swapchain->swapchain;

    u32 queue_family_indices[] = { static_cast<u32>(m_context.present_queue.family_index),
                                   static_cast<u32>(m_context.graphics_queue.family_index) };

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
            m_context.device, &swapchain_create_info, m_context.allocation_callbacks, &swapchain->swapchain),
        "Could not create swapchain");

    if (swapchain_create_info.oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_context.device, swapchain_create_info.oldSwapchain, m_context.allocation_callbacks);
    }

    {
        vkGetSwapchainImagesKHR(m_context.device, swapchain->swapchain, &swapchain->image_count, nullptr);
        VkImage* swapchain_images = m_frameAllocator->allocate_aligned<VkImage>(swapchain->image_count);
        vkGetSwapchainImagesKHR(m_context.device, swapchain->swapchain, &swapchain->image_count, swapchain_images);
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
                vkCreateImageView(
                    m_context.device, &image_view_create_info, m_context.allocation_callbacks, &image_views[i]),
                "Could not create image view");
        }
    }
}

void VulkanBackend::destroy_swapchain([[maybe_unused]] Handle swapchain_handle) {
    Swapchain& swapchain = m_context.swapchains[0];
    swapchain.window->get_event_handler().unbind_event(EventType::WindowResize, this);

    VkImageView* image_views = swapchain.image_views.get();

    for (u32 i = 0; i < swapchain.image_count; i++) {
        vkDestroyImageView(m_context.device, image_views[i], m_context.allocation_callbacks);
    }

    vkDestroySwapchainKHR(m_context.device, swapchain.swapchain, m_context.allocation_callbacks);
    swapchain.swapchain = VK_NULL_HANDLE;
    vkDestroySurfaceKHR(m_context.instance, swapchain.surface, m_context.allocation_callbacks);
    swapchain.surface = VK_NULL_HANDLE;
}

Handle VulkanBackend::create_render_pass() {
    TK_ASSERT(
        m_context.render_passes.size() <= MAX_RENDER_PASS_COUNT,
        "Creating a new render pass will exceed the maximum render pass count ({})",
        MAX_RENDER_PASS_COUNT);

    RenderPass render_pass{};
    render_pass.attachments.single.a0 = 0b01;
    render_pass.attachment_count = 1;
    Swapchain& swapchain = m_context.swapchains[0];

    VkAttachmentDescription* attachment_descriptions = m_frameAllocator->allocate_aligned<VkAttachmentDescription>(1);
    VkAttachmentReference* attachment_references = m_frameAllocator->allocate_aligned<VkAttachmentReference>(1);
    VkSubpassDependency* subpass_dependencies = m_frameAllocator->allocate_aligned<VkSubpassDependency>(1);

    for (u32 i = 0; i < 1; i++) {
        VkAttachmentDescription& attachment_description = attachment_descriptions[i] = {};
        attachment_description.format = swapchain.surface_format.format;
        attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachment_references[i] = {};
        attachment_references[i].attachment = i;
        attachment_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = attachment_references;
    subpass_description.inputAttachmentCount = 0;
    subpass_description.pInputAttachments = nullptr;
    subpass_description.pResolveAttachments = nullptr;
    subpass_description.pDepthStencilAttachment = nullptr;
    subpass_description.preserveAttachmentCount = 0;
    subpass_description.pPreserveAttachments = nullptr;

    VkSubpassDependency& subpass_dependency = subpass_dependencies[0];
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = attachment_descriptions;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = subpass_dependencies;

    VK_CHECK(
        vkCreateRenderPass(
            m_context.device, &render_pass_create_info, m_context.allocation_callbacks, &render_pass.render_pass),
        "Could not create render pass");

    render_pass.framebuffers = BasicRef<VkFramebuffer>{ &m_allocator, swapchain.image_count };

    Handle render_pass_handle = m_context.render_passes.emplace(render_pass);
    recreate_framebuffers(&render_pass, &swapchain, false);

    return render_pass_handle;
}

void VulkanBackend::destroy_render_pass(Handle render_pass_handle) {
    RenderPass& render_pass = m_context.render_passes[render_pass_handle];

    VkFramebuffer* framebuffers = render_pass.framebuffers.get();
    for (u32 i = 0; i < render_pass.framebuffers.size(); i++) {
        vkDestroyFramebuffer(m_context.device, framebuffers[i], m_context.allocation_callbacks);
    }

    vkDestroyRenderPass(m_context.device, render_pass.render_pass, m_context.allocation_callbacks);
}

void VulkanBackend::recreate_framebuffers(RenderPass* render_pass, Swapchain* swapchain, b8 destroy) {
    VkImageView* swapchain_image_views = swapchain->image_views.get();
    VkFramebuffer* framebuffers = render_pass->framebuffers.get();

    if (destroy) {
        for (u32 i = 0; i < render_pass->framebuffers.size(); i++) {
            vkDestroyFramebuffer(m_context.device, framebuffers[i], m_context.allocation_callbacks);
        }
    }

    VkFramebufferCreateInfo framebuffer_create_info{};
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.renderPass = render_pass->render_pass;
    framebuffer_create_info.attachmentCount = 1;
    framebuffer_create_info.width = swapchain->extent.width;
    framebuffer_create_info.height = swapchain->extent.height;
    framebuffer_create_info.layers = 1;

    for (u32 i = 0; i < swapchain->image_count; i++) {
        framebuffer_create_info.pAttachments = &swapchain_image_views[i];

        VK_CHECK(
            vkCreateFramebuffer(
                m_context.device, &framebuffer_create_info, m_context.allocation_callbacks, &framebuffers[i]),
            "Could not create framebuffer");
    }
}

PipelineResources VulkanBackend::create_pipeline_resources(const std::vector<configs::Shader>& stages) {
    PipelineResources resources{};

    std::vector<VkPushConstantRange> push_constants;

    for (const auto& stage : stages) {
        resources.binaries.emplace_back(stage.stage, create_shader_binary(stage));
        reflect_shader(stage.stage, resources.binaries.back().binary, resources.descriptor_bindings, push_constants);
    }

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pSetLayouts = nullptr;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;

    VK_CHECK(
        vkCreatePipelineLayout(
            m_context.device, &pipeline_layout_create_info, m_context.allocation_callbacks, &resources.pipeline_layout),
        "Could not create pipeline layout");

    return resources;
}

Shader VulkanBackend::create_pipeline([[maybe_unused]] Handle render_pass_handle, const configs::ShaderConfig& config) {
    TK_ASSERT(
        m_context.pipelines.size() <= MAX_PIPELINE_COUNT,
        "Creating a new pipeline will exceed the maximum pipeline count ({})",
        MAX_PIPELINE_COUNT);

    RenderPass& render_pass = *m_context.render_passes.begin();

    Pipeline pipeline{};

    VkShaderModule* shader_modules = m_frameAllocator->allocate_aligned<VkShaderModule>(config.stages.size());
    VkPipelineShaderStageCreateInfo* shader_stage_create_infos =
        m_frameAllocator->allocate_aligned<VkPipelineShaderStageCreateInfo>(config.stages.size());

    PipelineResources resources = create_pipeline_resources(config.stages);
    pipeline.pipeline_layout = resources.pipeline_layout;

    for (u32 i = 0; i < resources.binaries.size(); i++) {
        VkShaderModuleCreateInfo shader_module_create_info{};
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.codeSize = resources.binaries[i].binary.size() * 4;
        shader_module_create_info.pCode = resources.binaries[i].binary.data();

        VK_CHECK(
            vkCreateShaderModule(
                m_context.device, &shader_module_create_info, m_context.allocation_callbacks, &shader_modules[i]),
            "Could not create shader module");

        VkPipelineShaderStageCreateInfo& shader_stage_create_info = shader_stage_create_infos[i] = {};
        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.module = shader_modules[i];
        shader_stage_create_info.pName = "main";

        switch (resources.binaries[i].stage) {
            case ShaderStage::Vertex:
                shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case ShaderStage::Fragment:
                shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            default:
                std::unreachable();
        }
    }

    std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions(config.bindings.size());
    for (u32 i = 0; i < vertex_binding_descriptions.size(); i++) {
        vertex_binding_descriptions[i].binding = config.bindings[i].binding;
        vertex_binding_descriptions[i].stride = config.bindings[i].stride;

        switch (config.bindings[i].inputRate) {
            case VertexInputRate::VERTEX:
                vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                break;
            case VertexInputRate::INSTANCE:
                vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                break;
        }
    }

    std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions(config.attributes.size());
    for (u32 i = 0; i < vertex_attribute_descriptions.size(); i++) {
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
    vertex_input_state_create_info.vertexBindingDescriptionCount = vertex_binding_descriptions.size();
    vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
    vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

    switch (config.options.primitive_topology) {
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
        case PrimitiveTopology::PATCH_LIST:
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

    VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_state.blendEnable = VK_TRUE;  // TODO: enable blending
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states(1, color_blend_attachment_state);

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

    constexpr static VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = std::size(dynamic_states);
    dynamic_state_create_info.pDynamicStates = dynamic_states;

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
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
    graphics_pipeline_create_info.renderPass = render_pass.render_pass;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.layout = resources.pipeline_layout;

    TK_LOG_INFO("Creating new graphics pipeline");
    VK_CHECK(
        vkCreateGraphicsPipelines(
            m_context.device,
            VK_NULL_HANDLE,
            1,
            &graphics_pipeline_create_info,
            m_context.allocation_callbacks,
            &pipeline.pipeline),
        "Could not create graphics pipeline");

    for (u32 i = 0; i < resources.binaries.size(); i++) {
        vkDestroyShaderModule(m_context.device, shader_modules[i], m_context.allocation_callbacks);
    }

    return { m_context.pipelines.emplace(pipeline) };
}

void VulkanBackend::destroy_pipeline(Handle pipeline_handle) {
    Pipeline& pipeline = m_context.pipelines[pipeline_handle];
    vkDestroyPipeline(m_context.device, pipeline.pipeline, m_context.allocation_callbacks);
    pipeline.pipeline = VK_NULL_HANDLE;
    vkDestroyPipelineLayout(m_context.device, pipeline.pipeline_layout, m_context.allocation_callbacks);
    pipeline.pipeline_layout = VK_NULL_HANDLE;
}

std::vector<u32> VulkanBackend::create_shader_binary(configs::Shader shader) {
    std::string shader_source = loaders::read_text_file(shader.path);

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

    switch (shader.stage) {
        case ShaderStage::Vertex:
            shader_kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
            break;
        case ShaderStage::Fragment:
            shader_kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
            break;
        default:
            std::unreachable();
    }

    shaderc::SpvCompilationResult spirv_module =
        compiler.CompileGlslToSpv(shader_source.data(), shader_kind, ".", options);
    if (spirv_module.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
        TK_LOG_ERROR(
            "ERROR MESSAGE:\n{}\n\nCOMPILATION STATUS: {}\nSOURCE: {}",
            spirv_module.GetErrorMessage(),
            (int) spirv_module.GetCompilationStatus(),
            shader_source);
    }
    TK_ASSERT(
        spirv_module.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success,
        "Shader compilation error");

    TK_ASSERT(spirv_module.end() - spirv_module.begin() > 0, "");
    return std::vector<u32>{ spirv_module.begin(), spirv_module.end() };
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
        auto element_type = compiler.get_type(resources.push_constant_buffers[i].base_type_id);
        if (push_constants.size() == 1) {
            push_constants[i].stageFlags |= shader_stage;
        } else {
            VkPushConstantRange push_constant;
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
        m_context.device,
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
    command_buffer_allocate_info.commandPool = m_context.extra_command_pools[0];
    command_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(m_context.device, &command_buffer_allocate_info, &command_buffer);

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

    return command_buffer;
}

void VulkanBackend::submit_single_use_command_buffer(VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd;

    vkQueueSubmit(m_context.graphics_queue.handle, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_context.graphics_queue.handle);
}

FrameData* VulkanBackend::get_current_frame() {
    return &m_frames.data[m_inFlightFrameIndex];
}

CommandBuffers* VulkanBackend::get_current_command_buffers() {
    return &m_context.command_buffers[m_inFlightFrameIndex];
}

void VulkanBackend::wait_for_resources() {
    vkDeviceWaitIdle(m_context.device);
}

void VulkanBackend::prepare_frame_resources() {
    m_context.submit_count = 0;

    FrameData* frame = get_current_frame();

    VkResult result = vkWaitForFences(m_context.device, 1, &frame->render_fence, VK_TRUE, UINT64_MAX);
    TK_ASSERT(result == VK_SUCCESS || result == VK_TIMEOUT, "Failed waiting for fences");

    for (u32 i = 0; i < 1; i++) {
        prepare_swapchain_frame(&m_context.swapchains[i]);
    }

    VK_CHECK(vkResetFences(m_context.device, 1, &frame->render_fence), "Could not reset render fence");

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
    for (auto& swapchain : m_context.swapchains) {
        reset_swapchain_frame(&swapchain);
    }

    m_inFlightFrameIndex = (m_inFlightFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    m_frameAllocator.swap();
    m_frameAllocator->clear();
}

void VulkanBackend::present() {
    FrameData* frame = get_current_frame();
    VkSemaphore wait_semaphores[] = { frame->present_semaphore };

    u32 swapchain_count = 1;
    VkSwapchainKHR* swapchain_handles = m_frameAllocator->allocate_aligned<VkSwapchainKHR>(swapchain_count);
    u32* swapchain_image_indices = m_frameAllocator->allocate_aligned<u32>(swapchain_count);
    Swapchain** swapchains = m_frameAllocator->allocate_aligned<Swapchain*>(swapchain_count);

    swapchain_count = 0;

    for (auto& swapchain : m_context.swapchains) {
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

    VK_CHECK(vkQueuePresentKHR(m_context.present_queue.handle, &present_info), "Could not present");

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
        wait_semaphores[i] = m_context.swapchains[i].image_available_semaphores[m_inFlightFrameIndex];
    }

    CommandBuffers* command_buffers = get_current_command_buffers();

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pWaitDstStageMask = wait_stages;
    // TODO: make this dynamic when more than 1 swapchain is supported
    submit_info.waitSemaphoreCount = MAX_SWAPCHAIN_COUNT;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    submit_info.pCommandBuffers = command_buffers->command_buffers;
    submit_info.commandBufferCount = command_buffers->count;

    VK_CHECK(
        vkQueueSubmit(m_context.graphics_queue.handle, 1, &submit_info, current_frame->render_fence),
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
    m_context.color_clear = { { c.r, c.g, c.b, c.a } };
}

void VulkanBackend::set_depth_clear(f32 depth_clear) {
    m_context.depth_stencil_clear.depth = depth_clear;
}

void VulkanBackend::set_stencil_clear(u32 stencil_clear) {
    m_context.depth_stencil_clear.stencil = stencil_clear;
}

void VulkanBackend::begin_render_pass(VkCommandBuffer cmd, const Rect2D& render_area) {
    RenderPass render_pass = *m_context.render_passes.begin();

    VkClearValue clear_values[MAX_RENDER_PASS_ATTACHMENT_COUNT]{};
    for (u32 i = 0, value = render_pass.attachments.combined; i < render_pass.attachment_count; i++) {
        if ((value & 0b11) == 0b11) {  // Depth/Stencil
            clear_values[i].depthStencil = m_context.depth_stencil_clear;
        } else {
            clear_values[i].color = m_context.color_clear;
        }

        value >>= 2;
    }

    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = render_pass.render_pass;
    render_pass_begin_info.renderArea = *reinterpret_cast<const VkRect2D*>(&render_area);  // <-- very bad
    render_pass_begin_info.framebuffer = render_pass.framebuffers[m_context.swapchains[0].image_index];
    render_pass_begin_info.pClearValues = clear_values;
    render_pass_begin_info.clearValueCount = render_pass.attachment_count;

    vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanBackend::end_render_pass(VkCommandBuffer cmd) {
    vkCmdEndRenderPass(cmd);
}

void VulkanBackend::bind_shader(VkCommandBuffer cmd, Shader const& shader) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_context.pipelines[shader.handle].pipeline);
}

void VulkanBackend::bind_buffer(VkCommandBuffer cmd, Buffer const& buffer) {
    TK_ASSERT(m_context.buffers.contains(buffer.handle), "Buffer with provided handle does not exist");
    VulkanBuffer& internal_buffer = m_context.buffers[buffer.handle];

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
        m_context.staging_buffer = {};
        m_context.staging_buffer.internal_buffer_data = create_buffer_internal(
            DEFAULT_STAGING_BUFFER_SIZE,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    // Command pool creation
    {
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.queueFamilyIndex = m_context.graphics_queue.family_index;

        m_context.command_pools = BasicRef<VkCommandPool>(&m_allocator, 1);

        VK_CHECK(
            vkCreateCommandPool(
                m_context.device,
                &command_pool_create_info,
                m_context.allocation_callbacks,
                m_context.command_pools.get()),
            "Could not create command pool(s)");

        m_context.extra_command_pools = BasicRef<VkCommandPool>(&m_allocator, 1);

        VK_CHECK(
            vkCreateCommandPool(
                m_context.device,
                &command_pool_create_info,
                m_context.allocation_callbacks,
                m_context.extra_command_pools.get()),
            "Could not create extra command pool(s)");
    }

    // Command buffer creation
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = m_context.command_pools[0];
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = MAX_IN_FLIGHT_COMMAND_BUFFERS;

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_context.command_buffers[i].command_buffers = { &m_allocator, MAX_IN_FLIGHT_COMMAND_BUFFERS };
            m_context.command_buffers[i].count = 0;

            VK_CHECK(
                vkAllocateCommandBuffers(
                    m_context.device, &command_buffer_allocate_info, m_context.command_buffers[i].command_buffers),
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
                    m_context.device,
                    &fence_create_info,
                    m_context.allocation_callbacks,
                    &m_frames.data[i].render_fence),
                "Could not create render fence");
            VK_CHECK(
                vkCreateSemaphore(
                    m_context.device,
                    &semaphore_create_info,
                    m_context.allocation_callbacks,
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
            vkDestroyFence(m_context.device, m_frames.data[i].render_fence, m_context.allocation_callbacks);
            vkDestroySemaphore(m_context.device, m_frames.data[i].present_semaphore, m_context.allocation_callbacks);
        }
    }

    // Cleanup command pools
    {
        for (u32 i = 0; i < m_context.command_pools.size(); i++) {
            vkDestroyCommandPool(m_context.device, m_context.command_pools[i], m_context.allocation_callbacks);
        }

        for (u32 i = 0; i < m_context.extra_command_pools.size(); i++) {
            vkDestroyCommandPool(m_context.device, m_context.extra_command_pools[i], m_context.allocation_callbacks);
        }
    }

    // Cleanup internal buffers
    destroy_buffer_internal(&m_context.staging_buffer.internal_buffer_data);
}

Buffer VulkanBackend::create_buffer(BufferType type, u32 size) {
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

    Handle handle = m_context.buffers.emplace(create_buffer_internal(size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    Buffer buffer{ .type = type, .size = size, .handle = handle };

    return buffer;
}

void VulkanBackend::destroy_buffer(Buffer* buffer) {
    TK_ASSERT(buffer && buffer->size, "Invalid buffer provided");
    TK_ASSERT(m_context.buffers.contains(buffer->handle), "Buffer with provided handle does not exist");

    VulkanBuffer& internal_buffer = m_context.buffers[buffer->handle];
    destroy_buffer_internal(&internal_buffer);

    *buffer = {};
}

void* VulkanBackend::map_buffer_memory(VkDeviceMemory memory, u32 offset, u32 size) {
    void* data{};
    VK_CHECK(vkMapMemory(m_context.device, memory, offset, size, 0, &data), "Could not map buffer memory");
    return data;
}

void VulkanBackend::unmap_buffer_memory(VkDeviceMemory memory) {
    vkUnmapMemory(m_context.device, memory);
}

void VulkanBackend::flush_buffer(Buffer* buffer) {
    TK_ASSERT(m_context.buffers.contains(buffer->handle), "Buffer with provided handle does not exist");
    VulkanBuffer& internal_buffer = m_context.buffers[buffer->handle];

    if ((internal_buffer.memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
        VkMappedMemoryRange mapped_memory_range{};
        mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mapped_memory_range.memory = internal_buffer.memory;
        mapped_memory_range.offset = 0;
        mapped_memory_range.size = buffer->size;
        VK_CHECK(vkFlushMappedMemoryRanges(m_context.device, 1, &mapped_memory_range), "Could not flush buffer memory");
    }
}

void VulkanBackend::set_bufffer_data(Buffer* buffer, u32 size, void* data) {
    TK_ASSERT(m_context.buffers.contains(buffer->handle), "Buffer with provided handle does not exist");
    auto& staging_buffer = m_context.staging_buffer;

    // Copy to staging buffer
    void* mapped_data = map_buffer_memory(staging_buffer.internal_buffer_data.memory, staging_buffer.offset, size);
    std::memcpy(mapped_data, data, size);
    unmap_buffer_memory(staging_buffer.internal_buffer_data.memory);
    mapped_data = nullptr;

    VulkanBuffer& internal_buffer = m_context.buffers[buffer->handle];

    // Copy to uploaded buffer
    copy_buffer_data(
        internal_buffer.buffer, staging_buffer.internal_buffer_data.buffer, size, 0, staging_buffer.offset);

    staging_buffer.offset += size;
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

Handle VulkanBackend::create_image(ColorFormat format_in, u32 width, u32 height) {
    VkExtent3D extent = { width, height, 1 };
    VkFormat format = VK_FORMAT_UNDEFINED;
    switch (format_in) {
        case ColorFormat::RGBA8:
            format = VK_FORMAT_UNDEFINED;
            break;

        case ColorFormat::None:
            TK_ASSERT(false, "Color format not provided");

        default:
            TK_ASSERT(false, "TODO: add other formats");
    }

    VulkanImage new_image = create_image_internal(
        extent,
        format,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    return m_context.images.emplace(new_image);
}

void VulkanBackend::destroy_image(Handle image_handle) {
    TK_ASSERT(m_context.images.contains(image_handle), "Handle is not associated with any Vulkan image");
    m_context.images.erase(image_handle);
}

VulkanBuffer VulkanBackend::create_buffer_internal(
    u32 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties) {
    VulkanBuffer buffer{};

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = size;
    buffer_create_info.usage = usage;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(
        vkCreateBuffer(m_context.device, &buffer_create_info, m_context.allocation_callbacks, &buffer.buffer),
        "Could not create buffer");

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(m_context.device, buffer.buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex =
        find_memory_type_index(memory_requirements.memoryTypeBits, memory_properties);

    VK_CHECK(
        vkAllocateMemory(m_context.device, &memory_allocate_info, m_context.allocation_callbacks, &buffer.memory),
        "Could not allocate image memory");
    VK_CHECK(vkBindBufferMemory(m_context.device, buffer.buffer, buffer.memory, 0), "Could not bind buffer memory");

    buffer.usage = usage;
    buffer.memory_requirements = memory_requirements;
    buffer.memory_property_flags = memory_properties;

    return buffer;
}

void VulkanBackend::destroy_buffer_internal(VulkanBuffer* buffer) {
    vkFreeMemory(m_context.device, buffer->memory, m_context.allocation_callbacks);
    vkDestroyBuffer(m_context.device, buffer->buffer, m_context.allocation_callbacks);
    *buffer = {};
}

VulkanImage VulkanBackend::create_image_internal(
    VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties) {
    VulkanImage image;
    image.format = format;

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
        vkCreateImage(m_context.device, &image_create_info, m_context.allocation_callbacks, &image.image),
        "Could not create image");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(m_context.device, image.image, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex =
        find_memory_type_index(memory_requirements.memoryTypeBits, memory_properties);

    VK_CHECK(
        vkAllocateMemory(m_context.device, &memory_allocate_info, m_context.allocation_callbacks, &image.memory),
        "Could not allocate image memory");
    VK_CHECK(vkBindImageMemory(m_context.device, image.image, image.memory, 0), "Could not bind image memory");

    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(
        vkCreateImageView(m_context.device, &image_view_create_info, m_context.allocation_callbacks, &image.image_view),
        "Could not create image view");

    return image;
}

void VulkanBackend::destroy_image_internal(VulkanImage* image) {
    vkFreeMemory(m_context.device, image->memory, m_context.allocation_callbacks);
    vkDestroyImageView(m_context.device, image->image_view, m_context.allocation_callbacks);
    vkDestroyImage(m_context.device, image->image, m_context.allocation_callbacks);
    *image = {};
}

u32 VulkanBackend::find_memory_type_index(u32 type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(m_context.physical_device, &memory_properties);
    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    TK_ASSERT(false, "No correct memory type found");
    std::unreachable();
}

}  // namespace vulkan_renderer

}  // namespace toki
