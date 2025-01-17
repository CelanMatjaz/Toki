#include "vulkan_renderer.h"

#include "containers/handle_map.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vulkan/vulkan.h>

#include <print>
#include <utility>

#include "core/assert.h"
#include "core/defer.h"
#include "core/logging.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/platform/vulkan_platform.h"
#include "renderer/vulkan/vulkan_commands.h"
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

    create_default_resources();
}

#define DESTROY_AND_CLEAR(array)       \
    for (auto& v : m_context->array) { \
        v.destroy(m_context);          \
    }

VulkanRenderer::~VulkanRenderer() {
    TK_LOG_INFO("Shutting down renderer");

    vkDeviceWaitIdle(m_context->device);

    cleanup_default_resources();

    m_context->descriptor_pool_manager.destroy(m_context);

    for (auto& command_pool : m_context->command_pools) {
        vkDestroyCommandPool(m_context->device, command_pool, m_context->allocation_callbacks);
    }

    for (auto& command_pool : m_context->extra_command_pools) {
        vkDestroyCommandPool(m_context->device, command_pool, m_context->allocation_callbacks);
    }

    DESTROY_AND_CLEAR(buffers);
    DESTROY_AND_CLEAR(images);
    DESTROY_AND_CLEAR(shaders);
    DESTROY_AND_CLEAR(framebuffers);

    m_context->swapchain.destroy(m_context);

    vkDestroyDevice(m_context->device, m_context->allocation_callbacks);
    vkDestroyInstance(m_context->instance, m_context->allocation_callbacks);

    m_context.reset();
}

#undef DESTROY_AND_CLEAR

Handle VulkanRenderer::create_shader(const ShaderCreateConfig& config) {
    TK_ASSERT(m_context->framebuffers.contains(config.framebuffer_handle), "Cannot create shader with a non existing framebuffer");

    auto framebuffer = m_context->framebuffers.at(config.framebuffer_handle);
    VulkanGraphicsPipeline::Config graphics_pipeline_config{ .framebuffer = m_context->framebuffers.at(config.framebuffer_handle) };
    graphics_pipeline_config.shader_config = config.config;
    VulkanGraphicsPipeline pipeline{};
    pipeline.create(m_context, graphics_pipeline_config);
    return m_context->shaders.emplace(pipeline);
}

void VulkanRenderer::destroy_shader(Handle shader_handle) {
    TK_ASSERT(m_context->shaders.contains(shader_handle), "Shader with provided handle does not exist");
    m_context->shaders[shader_handle].destroy(m_context);
    m_context->shaders.erase(shader_handle);
}

Handle VulkanRenderer::create_buffer(const BufferCreateConfig& config) {
    TK_ASSERT(config.size > 0, "Cannot create a buffer of size 0");

    VulkanBuffer::Config create_buffer_config{};
    create_buffer_config.size = config.size;

    switch (config.type) {
        case BufferType::VERTEX:
            create_buffer_config.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case BufferType::INDEX:
            create_buffer_config.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case BufferType::UNIFORM:
            create_buffer_config.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        default:
            std::unreachable();
    }

    switch (config.usage) {
        case BufferUsage::STATIC:
            std::unreachable();  // TODO: need to implement staging buffers
        case BufferUsage::DYNAMIC:
            create_buffer_config.memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        default:
            std::unreachable();
    }

    VulkanBuffer buffer{};
    buffer.create(m_context, create_buffer_config);
    return m_context->buffers.emplace(buffer);
}

void VulkanRenderer::destroy_buffer(Handle buffer_handle) {
    TK_ASSERT(m_context->buffers.contains(buffer_handle), "Buffer with provided handle does not exist");
    m_context->buffers[buffer_handle].destroy(m_context);
    m_context->buffers.erase(buffer_handle);
}

Handle VulkanRenderer::create_texture(const TextureCreateConfig& config) {
    TK_ASSERT(config.size.x > 0, "Cannot create an image with width 0");
    TK_ASSERT(config.size.y > 0, "Cannot create an image with height 0");

    VulkanImage::Config create_image_config{};
    create_image_config.extent = { config.size.x, config.size.y, 1 };
    create_image_config.format = map_format(config.format);
    create_image_config.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    create_image_config.memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanImage image{};
    image.create(m_context, create_image_config);
    vulkan_commands::submit_single_use_command_buffer(m_context, [image](VkCommandBuffer cmd) mutable {
        image.transition_layout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });
    return m_context->images.emplace(image);
}

Handle VulkanRenderer::create_texture_from_file(std::string_view path) {
    int width, height, channels;
    int desired_channels = STBI_rgb_alpha;

    std::filesystem::path file_path(path);
    u32* pixels = (uint32_t*) stbi_load(file_path.string().c_str(), &width, &height, &channels, desired_channels);
    TK_ASSERT(pixels != nullptr, "Error loading image");
    u64 image_size = width * height * 4;
    TK_ASSERT(image_size > 0, "Image size is 0");

    TextureCreateConfig config{};
    config.format = ColorFormat::RGBA8;
    config.size = { width, height };
    Handle new_image = create_texture(config);
    set_texture_data(new_image, image_size, pixels);
    stbi_image_free(pixels);

    return new_image;
}

void VulkanRenderer::destroy_texture(Handle texture_handle) {
    TK_ASSERT(m_context->images.contains(texture_handle), "Texture with provided handle does not exist");
    m_context->images[texture_handle].destroy(m_context);
    m_context->images.erase(texture_handle);
}

Handle VulkanRenderer::create_framebuffer(const FramebufferCreateConfig& config) {
    TK_ASSERT(config.render_targets.size() <= MAX_FRAMEBUFFER_ATTACHMENTS, "Max {} render attachments supported, including depth and/or stencil", MAX_FRAMEBUFFER_ATTACHMENTS);

    VulkanFramebuffer framebuffer{};
    framebuffer.create(m_context, config);
    return m_context->framebuffers.emplace(framebuffer);
}

void VulkanRenderer::destroy_framebuffer(Handle framebuffer_handle) {
    TK_ASSERT(m_context->framebuffers.contains(framebuffer_handle), "Framebuffer with provided handle does not exist");
    m_context->framebuffers[framebuffer_handle].destroy(m_context);
    m_context->framebuffers.erase(framebuffer_handle);
}

void VulkanRenderer::set_buffer_data(Handle buffer_handle, u32 size, void* data) {
    TK_ASSERT(m_context->buffers.contains(buffer_handle), "Buffer with provided handle does not exist");
    m_context->buffers[buffer_handle].set_data(m_context, size, data);
}

void VulkanRenderer::set_texture_data(Handle texture_handle, u32 size, void* data) {
    TK_ASSERT(m_context->images.contains(texture_handle), "Texture with provided handle does not exist");
    m_context->images.at(texture_handle).set_data(m_context, size, data);
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
    Defer defer([ctx = m_context, surface]() {
        vkDestroySurfaceKHR(ctx->instance, surface, ctx->allocation_callbacks);
    });

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
    features.samplerAnisotropy = VK_TRUE;

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

void VulkanRenderer::create_descriptor_pools() {
    std::vector<VkDescriptorPoolSize> pool_sizes = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 } };
    m_context->descriptor_pool_manager.create(m_context, 256, pool_sizes);
}

void VulkanRenderer::create_default_resources() {
    m_context->framebuffers = containers::HandleMap<VulkanFramebuffer>(8);
    m_context->buffers = containers::HandleMap<VulkanBuffer>(256);
    m_context->images = containers::HandleMap<VulkanImage>(256);
    m_context->shaders = containers::HandleMap<VulkanGraphicsPipeline>(256);

    VkSamplerCreateInfo sampler_create_info{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.anisotropyEnable = VK_FALSE;
    sampler_create_info.maxAnisotropy = 1.0f;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = VK_LOD_CLAMP_NONE;

    VK_CHECK(vkCreateSampler(m_context->device, &sampler_create_info, m_context->allocation_callbacks, &m_context->default_sampler), "Could not create default sampler");
}

void VulkanRenderer::cleanup_default_resources() {
    vkDestroySampler(m_context->device, m_context->default_sampler, m_context->allocation_callbacks);
}

}  // namespace toki
