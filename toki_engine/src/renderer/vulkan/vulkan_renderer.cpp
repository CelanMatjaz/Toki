#include "vulkan_renderer.h"

#include <vulkan/vulkan.h>
#include <winnt.h>

#include <print>
#include <utility>

#include "core/assert.h"
#include "core/logging.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/platform/vulkan_platform.h"
#include "renderer/vulkan/vulkan_buffer.h"
#include "renderer/vulkan/vulkan_pipeline.h"
#include "renderer/vulkan/vulkan_renderer_api.h"
#include "renderer/vulkan/vulkan_swapchain.h"
#include "renderer/vulkan/vulkan_types.h"
#include "renderer/vulkan/vulkan_utils.h"
#include "vulkan/vulkan_core.h"

namespace toki {

vulkan_renderer::vulkan_renderer(const Config& config): renderer(config) {
    TK_LOG_INFO("Initializing renderer");
    _context = create_ref<renderer_context>();
    _renderer_api = create_ref<vulkan_renderer_api>(_context);

    create_instance();
    create_device(config.initialWindow);
    create_command_pools();

    create_vulkan_swapchain_config swapchain_config{};
    swapchain_config.window = config.initialWindow;
    swapchain_config.command_pool = _context->command_pools.front();
    _context->swapchain = vulkan_swapchain_create(_context, swapchain_config);
}

vulkan_renderer::~vulkan_renderer() {
    TK_LOG_INFO("Shutting down renderer");

    for (auto& command_pool : _context->command_pools) {
        vkDestroyCommandPool(_context->device, command_pool, _context->allocation_callbacks);
    }

    for (auto& command_pool : _context->extra_command_pools) {
        vkDestroyCommandPool(_context->device, command_pool, _context->allocation_callbacks);
    }

    for (auto& shader : _context->shaders) {
        vulkan_graphics_pipeline_destroy(_context, shader);
    }
    _context->shaders.clear();

    for (auto& buffer : _context->buffers) {
        vulkan_buffer_destroy(_context, buffer);
    }
    _context->buffers.clear();

    vulkan_swapchain_destroy(_context, _context->swapchain);

    vkDestroyDevice(_context->device, _context->allocation_callbacks);
    vkDestroyInstance(_context->instance, _context->allocation_callbacks);
}

handle vulkan_renderer::create_shader(const shader_create_config& config) {
    handle new_shader_handle = _context->shaders.size() + 1;

    create_graphics_pipeline_config create_graphics_pipeline_config{};
    create_graphics_pipeline_config.vertex_shader_path = config.vertex_shader_path;
    create_graphics_pipeline_config.fragment_shader_path = config.fragment_shader_path;
    create_graphics_pipeline_config.render_targets = config.render_targets;
    _context->shaders.emplace(new_shader_handle, vulkan_graphics_pipeline_create(_context, create_graphics_pipeline_config));

    return new_shader_handle;
}

void vulkan_renderer::destroy_shader(handle shader_handle) {
    TK_ASSERT(_context->shaders.contains(shader_handle), "Shader with provided handle does not exist");
    vulkan_graphics_pipeline_destroy(_context, _context->shaders[shader_handle]);
    _context->shaders.remove(shader_handle);
}

handle vulkan_renderer::create_buffer(const buffer_create_config& config) {
    TK_ASSERT(config.size > 0, "Cannot create a buffer of size 0");

    handle new_buffer_handle = _context->buffers.size() + 1;

    create_buffer_config create_buffer_config{};
    create_buffer_config.size = config.size;

    switch (config.type) {
        case buffer_type::VERTEX:
            create_buffer_config.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case buffer_type::INDEX:
            create_buffer_config.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        default:
            std::unreachable();
    }

    switch (config.usage) {
        case buffer_usage::STATIC:
        case buffer_usage::DYNAMIC:
            create_buffer_config.memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        default:
            std::unreachable();
    }

    _context->buffers.emplace(new_buffer_handle, vulkan_buffer_create(_context, create_buffer_config));

    return new_buffer_handle;
}

void vulkan_renderer::destroy_buffer(handle buffer_handle) {
    TK_ASSERT(_context->buffers.contains(buffer_handle), "Shader with provided handle does not exist");
    vulkan_buffer_destroy(_context, _context->buffers[buffer_handle]);
    _context->buffers.remove(buffer_handle);
}

b8 vulkan_renderer::begin_frame() {
    vulkan_swapchain& swapchain = _context->swapchain;
    frame& frame = swapchain.frames[swapchain.current_image_index];

    VkResult waitFencesResult = vkWaitForFences(_context->device, 1, &frame.render_fence, VK_TRUE, UINT64_MAX);
    TK_ASSERT(waitFencesResult == VK_SUCCESS || waitFencesResult == VK_TIMEOUT, "Failed waiting for fences");

    if (!vulkan_swapchain_start_recording(_context, swapchain)) {
        return false;
    }

    VK_CHECK(vkResetFences(_context->device, 1, &frame.render_fence), "Could not reset fence");

    return true;
}

void vulkan_renderer::end_frame() {
    vulkan_swapchain swapchain = _context->swapchain;
    vulkan_swapchain_stop_recording(_context, swapchain);
}

void vulkan_renderer::present() {
    vulkan_swapchain& swapchain = _context->swapchain;
    frame& frame = swapchain.frames[swapchain.current_image_index];

    VkSemaphore wait_semaphores[] = { frame.present_semaphore };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signal_semaphores[] = { frame.present_semaphore };

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    submit_info.pCommandBuffers = &frame.command.handle;
    submit_info.commandBufferCount = 1;

    VK_CHECK(vkQueueSubmit(_context->queues.graphics, 1, &submit_info, frame.render_fence), "Could not submit for rendering");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signal_semaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain.handle;
    presentInfo.pImageIndices = &swapchain.current_image_index;

    VkResult result = vkQueuePresentKHR(_context->queues.present, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        vulkan_swapchain_recreate(_context, swapchain);
        const auto& [width, height] = swapchain.extent;
        return;
    }

    swapchain.current_frame = (swapchain.current_frame + 1) % swapchain.image_count;

    VkResult waitFencesResult = vkWaitForFences(_context->device, 1, &frame.render_fence, VK_TRUE, UINT64_MAX);
    TK_ASSERT(waitFencesResult == VK_SUCCESS || waitFencesResult == VK_TIMEOUT, "Failed waiting for fences");
}

void vulkan_renderer::create_instance() {
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "Toki";
    application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    application_info.pEngineName = "Toki Engine";
    application_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_3;

    uint32_t extensionCount = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.enabledExtensionCount = extensionCount;
    instance_create_info.ppEnabledExtensionNames = extensions;

#ifndef TK_DIST
    TK_ASSERT(check_validation_layer_support(), "Validation layers not supported");
    instance_create_info.enabledLayerCount = validation_layers.size();
    instance_create_info.ppEnabledLayerNames = validation_layers.data();
#endif

    TK_LOG_INFO("Creating new Vulkan instance");
    VK_CHECK(vkCreateInstance(&instance_create_info, _context->allocation_callbacks, &_context->instance), "Could not initialize renderer");
}

void vulkan_renderer::create_device(ref<window> window) {
    VkSurfaceKHR surface = create_surface(_context, reinterpret_cast<GLFWwindow*>(window->get_handle()));
    Scoped<VkSurfaceKHR, VK_NULL_HANDLE> temp_surface(surface, [ctx = _context](VkSurfaceKHR s) { vkDestroySurfaceKHR(ctx->instance, s, ctx->allocation_callbacks); });

    u32 physical_device_count{};
    vkEnumeratePhysicalDevices(_context->instance, &physical_device_count, nullptr);
    TK_ASSERT(physical_device_count > 0, "No GPUs found");
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(_context->instance, &physical_device_count, physical_devices.data());

    u32 device_index = 0;
    for (u32 i = 0; i < physical_devices.size(); i++) {
        bool is_suitable = is_device_suitable(physical_devices[i]);
        if (is_suitable) {
            device_index = i;
            break;
        } else if (i == physical_devices.size() - 1) {
            TK_ASSERT(false, "No suitable GPU found");
        }
    }

    VkPhysicalDevice physical_device = _context->physical_device = physical_devices[device_index];

    const auto [graphics, present, transfer] = _context->queue_family_indices = find_queue_families(physical_device, temp_surface);
    TK_ASSERT(graphics != -1, "Graphics family index not found");
    TK_ASSERT(present != -1, "Present family index not found");
    TK_ASSERT(transfer != -1, "Transfer family index not found");

    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_infos[3]{};
    queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[0].queueFamilyIndex = graphics;
    queue_create_infos[0].queueCount = 1;
    queue_create_infos[0].pQueuePriorities = &queue_priority;
    queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[1].queueFamilyIndex = present;
    queue_create_infos[1].queueCount = 1;
    queue_create_infos[1].pQueuePriorities = &queue_priority;
    queue_create_infos[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[2].queueFamilyIndex = transfer;
    queue_create_infos[2].queueCount = 1;
    queue_create_infos[2].pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures2 device_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };

    // Extra features
    {
        VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_feature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES };
        dynamic_rendering_feature.dynamicRendering = VK_TRUE;
        device_features.pNext = &dynamic_rendering_feature;
    }

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &device_features;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.queueCreateInfoCount = graphics == present ? 1 : 3;
    device_create_info.enabledExtensionCount = vulkan_extensions.size();
    device_create_info.ppEnabledExtensionNames = vulkan_extensions.data();

    TK_LOG_INFO("Creating new Vulkan device");
    VK_CHECK(vkCreateDevice(_context->physical_device, &device_create_info, _context->allocation_callbacks, &_context->device), "Could not create device");

    vkGetDeviceQueue(_context->device, graphics, 0, &_context->queues.graphics);
    vkGetDeviceQueue(_context->device, present, 0, &_context->queues.present);
    vkGetDeviceQueue(_context->device, transfer, 0, &_context->queues.transfer);
}

void vulkan_renderer::create_command_pools() {
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = _context->queue_family_indices.graphics;

    VkCommandPool command_pool;
    VK_CHECK(vkCreateCommandPool(_context->device, &command_pool_create_info, _context->allocation_callbacks, &command_pool), "Could not create command pool");
    _context->command_pools.emplace_back(command_pool);

    VK_CHECK(vkCreateCommandPool(_context->device, &command_pool_create_info, _context->allocation_callbacks, &command_pool), "Could not create extra command pool");
    _context->extra_command_pools.emplace_back(command_pool);
}

}  // namespace toki
