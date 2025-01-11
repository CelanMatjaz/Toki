#include "vulkan_renderer.h"

#include <vulkan/vulkan.h>

#include <print>
#include <utility>

#include "core/assert.h"
#include "core/defer.h"
#include "core/logging.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/platform/vulkan_platform.h"
#include "renderer/vulkan/state/vulkan_state.h"
#include "renderer/vulkan/vulkan_renderer_api.h"
#include "renderer/vulkan/vulkan_utils.h"

namespace toki {

VulkanRenderer::VulkanRenderer(const Config& config): Renderer(config) {
    TK_LOG_INFO("Initializing renderer");
    m_context = create_ref<RendererContext>();
    m_rendererApi = create_ref<VulkanRendererApi>(m_context);

    create_instance();
    create_device(config.initialWindow);
    create_command_pools();

    VulkanSwapchain::Config swapchain_config{};
    swapchain_config.window = config.initialWindow;
    swapchain_config.command_pool = m_context->command_pools.front();
    m_context->swapchain.create(m_context, swapchain_config);
}

VulkanRenderer::~VulkanRenderer() {
    TK_LOG_INFO("Shutting down renderer");

    vkDeviceWaitIdle(m_context->device);

    for (auto& command_pool : m_context->command_pools) {
        vkDestroyCommandPool(m_context->device, command_pool, m_context->allocation_callbacks);
    }

    for (auto& command_pool : m_context->extra_command_pools) {
        vkDestroyCommandPool(m_context->device, command_pool, m_context->allocation_callbacks);
    }

    for (auto& shader : m_context->shaders) {
        shader.destroy(m_context);
    }
    m_context->shaders.clear();

    for (auto& buffer : m_context->buffers) {
        buffer.destroy(m_context);
    }
    m_context->buffers.clear();

    m_context->swapchain.destroy(m_context);

    vkDestroyDevice(m_context->device, m_context->allocation_callbacks);
    vkDestroyInstance(m_context->instance, m_context->allocation_callbacks);
}

Handle VulkanRenderer::create_shader(const ShaderCreateConfig& config) {
    TK_ASSERT(m_context->framebuffers.contains(config.framebuffer_handle), "Cannot create shader with a non existing framebuffer");

    Handle new_shader_handle = m_context->shaders.size() + 1;

    auto framebuffer = m_context->framebuffers.get(config.framebuffer_handle);
    VulkanGraphicsPipeline::Config graphics_pipeline_config{ .framebuffer = m_context->framebuffers.get(config.framebuffer_handle) };
    graphics_pipeline_config.shader_config = config.config;
    VulkanGraphicsPipeline pipeline{};
    pipeline.create(m_context, graphics_pipeline_config);
    m_context->shaders.emplace(new_shader_handle, pipeline);
    return new_shader_handle;
}

void VulkanRenderer::destroy_shader(Handle shader_handle) {
    TK_ASSERT(m_context->shaders.contains(shader_handle), "Shader with provided handle does not exist");
    m_context->shaders[shader_handle].destroy(m_context);
    m_context->shaders.remove(shader_handle);
}

Handle VulkanRenderer::create_buffer(const BufferCreateConfig& config) {
    TK_ASSERT(config.size > 0, "Cannot create a buffer of size 0");

    Handle new_buffer_handle = m_context->buffers.size() + 1;

    VulkanBuffer::Config create_buffer_config{};
    create_buffer_config.size = config.size;

    switch (config.type) {
        case BufferType::VERTEX:
            create_buffer_config.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case BufferType::INDEX:
            create_buffer_config.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        default:
            std::unreachable();
    }

    switch (config.usage) {
        case BufferUsage::STATIC:
        case BufferUsage::DYNAMIC:
            create_buffer_config.memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        default:
            std::unreachable();
    }

    VulkanBuffer buffer{};
    buffer.create(m_context, create_buffer_config);
    m_context->buffers.emplace(new_buffer_handle, buffer);

    return new_buffer_handle;
}

void VulkanRenderer::destroy_buffer(Handle buffer_handle) {
    TK_ASSERT(m_context->buffers.contains(buffer_handle), "Shader with provided handle does not exist");
    m_context->buffers[buffer_handle].destroy(m_context);
    m_context->buffers.remove(buffer_handle);
}

Handle VulkanRenderer::create_framebuffer(const FramebufferCreateConfig& config) {
    Handle handle = m_context->framebuffers.size() + 1;

    VulkanFramebuffer::Config framebuffer_config{};
    framebuffer_config.render_targets = config.render_targets;

    VulkanFramebuffer framebuffer{};
    framebuffer.create(m_context, framebuffer_config);
    m_context->framebuffers.emplace(handle, framebuffer);

    return handle;
}

void VulkanRenderer::destroy_framebuffer(Handle framebuffer_handle) {
    TK_ASSERT(m_context->framebuffers.contains(framebuffer_handle), "Framebuffer with provided handle does not exist");
    m_context->framebuffers[framebuffer_handle].destroy(m_context);
    m_context->framebuffers.remove(framebuffer_handle);
}

void VulkanRenderer::set_buffer_data(Handle buffer_handle, u32 size, void* data) {
    TK_ASSERT(m_context->buffers.contains(buffer_handle), "Buffer with provided handle does not exist");
    m_context->buffers[buffer_handle].set_data(m_context, size, data);
}

b8 VulkanRenderer::begin_frame() {
    VulkanSwapchain& swapchain = m_context->swapchain;

    if (!swapchain.start_recording(m_context)) {
        return false;
    }

    return true;
}

void VulkanRenderer::end_frame() {
    VulkanSwapchain& swapchain = m_context->swapchain;
    swapchain.transition_current_frame_image();
    swapchain.stop_recording(m_context);
}

void VulkanRenderer::present() {
    VulkanSwapchain& swapchain = m_context->swapchain;
    Frame& frame = swapchain.get_current_frame();

    VkSemaphore wait_semaphores[] = { frame.image_available_semaphore };
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

    VK_CHECK(vkQueueSubmit(m_context->queues.graphics, 1, &submit_info, frame.render_fence), "Could not submit for rendering");

    VkSwapchainKHR swapchain_handles[] = { swapchain.get_handle() };
    u32 swapchain_indices[] = { swapchain.get_current_image_index() };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signal_semaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchain_handles;
    presentInfo.pImageIndices = swapchain_indices;

    VkResult result = vkQueuePresentKHR(m_context->queues.present, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        swapchain.recreate(m_context);
    }

    swapchain.end_frame(m_context);
}

void VulkanRenderer::wait_for_resources() {
    vkDeviceWaitIdle(m_context->device);
}

void VulkanRenderer::create_instance() {
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
    TK_ASSERT(check_validation_layer_support(), "Validation layers not supported");
    instance_create_info.enabledLayerCount = validation_layers.size();
    instance_create_info.ppEnabledLayerNames = validation_layers.data();
#endif

    instance_create_info.enabledExtensionCount = extensions.size();
    instance_create_info.ppEnabledExtensionNames = extensions.data();

    TK_LOG_INFO("Creating new Vulkan instance");
    VK_CHECK(vkCreateInstance(&instance_create_info, m_context->allocation_callbacks, &m_context->instance), "Could not initialize renderer");
}

void VulkanRenderer::create_device(Ref<Window> window) {
    VkSurfaceKHR surface = create_surface(m_context, reinterpret_cast<GLFWwindow*>(window->get_handle()));
    Defer defer([ctx = m_context, surface]() { vkDestroySurfaceKHR(ctx->instance, surface, ctx->allocation_callbacks); });

    u32 physical_device_count{};
    vkEnumeratePhysicalDevices(m_context->instance, &physical_device_count, nullptr);
    TK_ASSERT(physical_device_count > 0, "No GPUs found");
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(m_context->instance, &physical_device_count, physical_devices.data());

    u32 device_index = 0;
    for (u32 i = 0; i < physical_devices.size(); i++) {
        b8 is_suitable = is_device_suitable(physical_devices[i]);
        if (is_suitable) {
            device_index = i;
            break;
        } else if (i == physical_devices.size() - 1) {
            TK_ASSERT(false, "No suitable GPU found");
        }
    }

    VkPhysicalDevice physical_device = m_context->physical_device = physical_devices[device_index];
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    TK_LOG_INFO("GPU name: {}", properties.deviceName);

    const auto [graphics, present, transfer] = m_context->queue_family_indices = find_queue_families(physical_device, surface);
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

    // Extra features
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_feature{};
    dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamic_rendering_feature.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceFeatures features{};
    features.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &dynamic_rendering_feature;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.enabledExtensionCount = vulkan_extensions.size();
    device_create_info.ppEnabledExtensionNames = vulkan_extensions.data();
    device_create_info.pEnabledFeatures = &features;

    TK_LOG_INFO("Creating new Vulkan device");
    VK_CHECK(vkCreateDevice(m_context->physical_device, &device_create_info, m_context->allocation_callbacks, &m_context->device), "Could not create device");

    vkGetDeviceQueue(m_context->device, graphics, 0, &m_context->queues.graphics);
    vkGetDeviceQueue(m_context->device, present, 0, &m_context->queues.present);
    vkGetDeviceQueue(m_context->device, transfer, 0, &m_context->queues.transfer);
}

void VulkanRenderer::create_command_pools() {
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = m_context->queue_family_indices.graphics;

    VkCommandPool command_pool;
    VK_CHECK(vkCreateCommandPool(m_context->device, &command_pool_create_info, m_context->allocation_callbacks, &command_pool), "Could not create command pool");
    m_context->command_pools.emplace_back(command_pool);

    VK_CHECK(vkCreateCommandPool(m_context->device, &command_pool_create_info, m_context->allocation_callbacks, &command_pool), "Could not create extra command pool");
    m_context->extra_command_pools.emplace_back(command_pool);
}

}  // namespace toki
