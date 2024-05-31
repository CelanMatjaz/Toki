#include "vulkan_renderer.h"

#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <spirv_cross/spirv_cross.hpp>
#include <unordered_map>

#include "platform.h"
#include "renderer/buffer.h"
#include "renderer/renderer_state.h"
#include "renderer/vulkan_types.h"
#include "toki/core/assert.h"
#include "toki/core/core.h"
#include "toki/events/event.h"

#ifdef TK_WINDOW_SYSTEM_GLFW
#include "GLFW/glfw3.h"
#endif

static const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

static const std::vector<const char*> requiredExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
};

namespace Toki {

static bool checkDeviceExtensionSupport();
static bool checkValidationLayerSupport();
static void createInstance();
static void queryPhysicalDevice();
static void createDevice();
static void createCommandPools();
static void createDescriptorPools();
static void initFrames();
static void initResources();

static VkSurfaceFormatKHR findSurfaceFormat();
static VkPresentModeKHR findPresentMode();

[[nodiscard]] static VkSurfaceKHR createSurface(Ref<Window> window) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;

#ifdef TK_WINDOW_SYSTEM_GLFW
    TK_ASSERT_VK_RESULT(glfwCreateWindowSurface(context.instance, (GLFWwindow*) window->getHandle(), nullptr, &surface), "Could not create surface");
#endif

    TK_ASSERT(surface != VK_NULL_HANDLE, "Surface should not be VK_NULL_HANDLE");

    return surface;
}

static void createNewGeometryVertexBuffer() {
    s_geometryVertexBuffers.emplace_back(createScope<Buffer>(
        DEFAULT_GEOMETRY_VERTEX_BUFFER_SIZE,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    ++s_currentGeometryVertexBufferIndex;
}

static void createNewGeometryIndexBuffer() {
    s_geometryIndexBuffers.emplace_back(createScope<Buffer>(
        DEFAULT_GEOMETRY_VERTEX_BUFFER_SIZE,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    ++s_currentGeometryIndexBufferIndex;
}

void VulkanRenderer::init() {
    createInstance();
    queryPhysicalDevice();
    createDevice();
    createCommandPools();
    createDescriptorPools();
    initFrames();

    initResources();

    Event::bindEvent(EventType::WindowResize, this, [this](void* sender, void* receiver, const Event& event) { m_wasWindowResized = true; });
}

void VulkanRenderer::shutdown() {
    vkDeviceWaitIdle(context.device);

    s_currentlyBoundPipeline.reset();

    s_geometryVertexBuffers.clear();
    s_geometryIndexBuffers.clear();

    s_bufferMap.clear();
    s_textureMap.clear();
    s_pipelineMap.clear();
    s_framebufferMap.clear();
    s_renderPassMap.clear();
    s_samplerMap.clear();
    s_swapchainMap.clear();

    for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
        vkDestroyFence(context.device, s_frameContext[i].renderFence, context.allocationCallbacks);
        vkDestroySemaphore(context.device, s_frameContext[i].renderSemaphore, context.allocationCallbacks);
        vkDestroySemaphore(context.device, s_frameContext[i].presentSemaphore, context.allocationCallbacks);
    }

    for (const auto& [_, swapchain] : s_swapchainMap) {
        vkDestroySwapchainKHR(context.device, swapchain->m_swapchain, context.allocationCallbacks);
        vkDestroySurfaceKHR(context.instance, swapchain->m_surface, context.allocationCallbacks);
    }

    vkDestroyDescriptorPool(context.device, s_descriptorPool, context.allocationCallbacks);

    s_commandPools.clear();
    s_extraCommandPools.clear();

    vkDestroyDevice(context.device, context.allocationCallbacks);
    vkDestroyInstance(context.instance, context.allocationCallbacks);
}

void VulkanRenderer::bindWindow(Ref<Window> window) {
    Handle handle;
    s_swapchainMap.emplace(handle, createRef<Swapchain>(createSurface(window)));
}

void VulkanRenderer::beginFrame([[maybe_unused]] const FrameData& data) {
    FrameContext& frameContext = s_frameContext[s_currentFrameIndex];

    static uint64_t timeout = UINT64_MAX - 1;

    for (auto& [_, swapchain] : s_swapchainMap) {
        swapchain->m_isRendering = true;
        VkResult result =
            vkAcquireNextImageKHR(context.device, swapchain->m_swapchain, timeout, frameContext.presentSemaphore, nullptr, &swapchain->m_imageIndex);

        VkResult waitFencesResult = vkWaitForFences(context.device, 1, &frameContext.renderFence, VK_TRUE, timeout);
        TK_ASSERT(waitFencesResult == VK_SUCCESS || waitFencesResult == VK_TIMEOUT, "Failed waiting for fences");

        if (result == VK_ERROR_OUT_OF_DATE_KHR || m_wasWindowResized) {
            swapchain->recreate();
            m_wasWindowResized = false;

            const auto& [width, height] = swapchain->m_extent;
            for (auto& [_, framebuffer] : s_framebufferMap) {
                framebuffer->recreate(Point3D{ width, height, 1 });
            }

            swapchain->m_isRendering = false;
            s_commandPools[s_currentFrameIndex].resetCommandBuffers();

            return;

        } else {
            TK_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire swapchain image");
        }

        break;  // Currently, only 1 swapchain is supported
    }

    TK_ASSERT_VK_RESULT(vkResetFences(context.device, 1, &frameContext.renderFence), "Error resetting fences");

    s_commandPools[s_currentFrameIndex].resetCommandBuffers();
    s_commandPools[s_currentFrameIndex].beginCommandBuffers();
}

void VulkanRenderer::endFrame([[maybe_unused]] const FrameData& data) {
    if (!s_swapchainMap.begin()->second->m_isRendering) {
        return;
    }

    s_commandPools[s_currentFrameIndex].endCommandBuffers();
}

void VulkanRenderer::present([[maybe_unused]] const FrameData& data) {
    if (!s_swapchainMap.begin()->second->m_isRendering) {
        return;
    }

    FrameContext& frameContext = s_frameContext[s_currentFrameIndex];

    std::vector<VkSwapchainKHR> swapchainHandles(s_swapchainMap.size());
    std::vector<uint32_t> swapchainIndices(s_swapchainMap.size());

    for (uint32_t i = 0; const auto& [_, swapchain] : s_swapchainMap) {
        swapchainHandles[i] = swapchain->m_swapchain;
        swapchainIndices[i] = swapchain->m_imageIndex;
        ++i;
    }

    VkSemaphore waitSemaphores[] = { frameContext.presentSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { frameContext.presentSemaphore };

    std::vector<VkSubmitInfo> submitInfos;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    /* for (uint32_t i = 0; i < MAX_FRAMES; ++i) */ {
        CommandPool& commandPool = s_commandPools[s_currentFrameIndex];

        commandPool.setSubmittableCommandBuffers();

        if (const auto& commandBuffers = commandPool.getSubmittableCommandBuffers(); commandBuffers.size() > 0) {
            submitInfo.commandBufferCount = commandBuffers.size();
            submitInfo.pCommandBuffers = commandBuffers.data();
            submitInfos.emplace_back(submitInfo);
        }
    }

    TK_ASSERT_VK_RESULT(
        vkQueueSubmit(context.physicalDeviceData.graphicsQueue, submitInfos.size(), submitInfos.data(), frameContext.renderFence),
        "Could not submit for rendering");

    std::vector<VkResult> results(s_swapchainMap.size());

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = swapchainHandles.size();
    presentInfo.pSwapchains = swapchainHandles.data();
    presentInfo.pImageIndices = swapchainIndices.data();
    presentInfo.pResults = results.data();

    TK_ASSERT_VK_RESULT(vkQueuePresentKHR(context.physicalDeviceData.presentQueue, &presentInfo), "Could not submit for presenting");

    for (/* uint32_t i = 0; */ auto& result : results) {
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_wasWindowResized) {
            s_swapchainMap.begin()->second->recreate();
            m_wasWindowResized = false;

            const auto& [width, height] = s_swapchainMap.begin()->second->m_extent;
            for (auto& [_, framebuffer] : s_framebufferMap) {
                framebuffer->recreate(Point3D{ width, height, 1 });
            }
        }

        // ++i;
        break;
    }

    s_currentFrameIndex = (s_currentFrameIndex + 1) % MAX_FRAMES;

    VkResult waitFencesResult = vkWaitForFences(context.device, 1, &frameContext.renderFence, VK_TRUE, UINT64_MAX);
    TK_ASSERT(waitFencesResult == VK_SUCCESS || waitFencesResult == VK_TIMEOUT, "Failed waiting for fences");
}

static void createInstance() {
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Toki";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "Toki";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

#ifdef TK_WINDOW_SYSTEM_GLFW
    uint32_t extensionCount = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
#endif

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = extensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = extensions;

#ifndef TK_DIST
    if (checkValidationLayerSupport()) {
        instanceCreateInfo.enabledLayerCount = validationLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        TK_ASSERT(false, "1 or more validation layers not supported by the system");
    }
#endif  // !DIST

    TK_ASSERT_VK_RESULT(vkCreateInstance(&instanceCreateInfo, context.allocationCallbacks, &context.instance), "Could not create instance");
}

static void queryPhysicalDevice() {
    WindowConfig windowConfig{};
    windowConfig.showOnCreate = false;
    Ref<Window> tempWindow = Window::create(windowConfig);
    VkSurfaceKHR SURFACE_TEMP = createSurface(tempWindow);

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(context.instance, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(context.instance, &physicalDeviceCount, physicalDevices.data());
    context.physicalDeviceData.physicalDevice = physicalDevices[0];

    PhysicalDeviceData& physicalDeviceData = context.physicalDeviceData;
    VkPhysicalDevice physicalDevice = context.physicalDeviceData.physicalDevice;

    vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceData.physicalDeviceFeatures);
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceData.physicalDeviceProperties);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceData.memoryProperties);

    {
        // Query queue families
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        physicalDeviceData.queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, physicalDeviceData.queueFamilyProperties.data());

        // Query present support for queue families
        physicalDeviceData.supportsPresent.resize(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, SURFACE_TEMP, &physicalDeviceData.supportsPresent[i]);

            if (physicalDeviceData.supportsPresent[i] && !physicalDeviceData.presentFamilyIndex.has_value()) {
                physicalDeviceData.presentFamilyIndex = i;
            }

            if ((physicalDeviceData.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                !physicalDeviceData.graphicsFamilyIndex.has_value()) {
                physicalDeviceData.graphicsFamilyIndex = i;
            }

            if ((physicalDeviceData.queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                !physicalDeviceData.transferFamilyIndex.has_value()) {
                physicalDeviceData.transferFamilyIndex = i;
            }
        }

        TK_ASSERT(physicalDeviceData.presentFamilyIndex.has_value(), "Physical device does not support present queue");
        TK_ASSERT(physicalDeviceData.graphicsFamilyIndex.has_value(), "Physical device does not support graphics queue");
        TK_ASSERT(physicalDeviceData.transferFamilyIndex.has_value(), "Physical device does not support a dedicated transfer queue");
    }

    {
        // Query present modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, SURFACE_TEMP, &presentModeCount, nullptr);
        physicalDeviceData.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, SURFACE_TEMP, &presentModeCount, physicalDeviceData.presentModes.data());
    }

    {
        // Query surface formats
        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, SURFACE_TEMP, &surfaceFormatCount, nullptr);
        physicalDeviceData.surfaceFormats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, SURFACE_TEMP, &surfaceFormatCount, physicalDeviceData.surfaceFormats.data());
    }

    physicalDeviceData.presentableSurfaceFormat = findSurfaceFormat();
    physicalDeviceData.presentMode = findPresentMode();

    TK_ASSERT(
        physicalDeviceData.physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, "Physical device is not a discrete GPU");
    // TK_ASSERT(physicalDeviceData.physicalDeviceProperties.apiVersion == VK_API_VERSION_1_3, "Physical device does not support Vulkan vertion 1.3");

    vkDestroySurfaceKHR(context.instance, SURFACE_TEMP, context.allocationCallbacks);
};

static void createDevice() {
    float queuePriority = 1.0f;
    uint32_t queueFamilies[] = { context.physicalDeviceData.graphicsFamilyIndex.value(),
                                 context.physicalDeviceData.presentFamilyIndex.value(),
                                 context.physicalDeviceData.transferFamilyIndex.value() };
    VkDeviceQueueCreateInfo deviceQueueCreateInfos[sizeof(queueFamilies) / sizeof(uint32_t)] = {};

    for (uint32_t i = 0; i < sizeof(queueFamilies) / sizeof(uint32_t); ++i) {
        deviceQueueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfos[i].queueFamilyIndex = queueFamilies[i];
        deviceQueueCreateInfos[i].queueCount = 1;
        deviceQueueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    physicalDeviceFeatures.fillModeNonSolid = true;
    physicalDeviceFeatures.wideLines = true;
    physicalDeviceFeatures.multiViewport = true;
    physicalDeviceFeatures.samplerAnisotropy = true;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = &dynamicRenderingFeatures;
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    deviceCreateInfo.enabledExtensionCount = requiredExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
    deviceCreateInfo.queueCreateInfoCount =
        context.physicalDeviceData.graphicsFamilyIndex.value() == context.physicalDeviceData.presentFamilyIndex.value() ? 1 : 2;

    TK_ASSERT_VK_RESULT(vkCreateDevice(context, &deviceCreateInfo, context.allocationCallbacks, &context.device), "Could not create device");

    vkGetDeviceQueue(context.device, context.physicalDeviceData.graphicsFamilyIndex.value(), 0, &context.physicalDeviceData.graphicsQueue);
    vkGetDeviceQueue(context.device, context.physicalDeviceData.presentFamilyIndex.value(), 0, &context.physicalDeviceData.presentQueue);
    vkGetDeviceQueue(context.device, context.physicalDeviceData.transferFamilyIndex.value(), 0, &context.physicalDeviceData.transferQueue);
}

static void createCommandPools() {
    s_commandPools.resize(s_renderThreadCount * MAX_FRAMES);

    for (auto& pool : s_commandPools) {
        pool.allocateCommandBuffer();
    }

    s_extraCommandPools.resize(EXTRA_COMMAND_POOL_COUNT);
}

static void createDescriptorPools() {
    std::vector<VkDescriptorPoolSize> poolSizes = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
                                                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
                                                    { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
                                                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 } };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = MAX_DESCRIPTOR_SETS;
    descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

    TK_ASSERT_VK_RESULT(
        vkCreateDescriptorPool(context.device, &descriptorPoolCreateInfo, context.allocationCallbacks, &s_descriptorPool),
        "Could not create ddescriptor pool");
}

static void initFrames() {
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
        TK_ASSERT_VK_RESULT(
            vkCreateFence(context.device, &fenceCreateInfo, context.allocationCallbacks, &s_frameContext[i].renderFence),
            "Could not create render fence");
        TK_ASSERT_VK_RESULT(
            vkCreateSemaphore(context.device, &semaphoreCreateInfo, context.allocationCallbacks, &s_frameContext[i].renderSemaphore),
            "Could not create render semaphore");
        TK_ASSERT_VK_RESULT(
            vkCreateSemaphore(context.device, &semaphoreCreateInfo, context.allocationCallbacks, &s_frameContext[i].presentSemaphore),
            "Could not create present semaphore");

        // VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        // commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        // commandBufferAllocateInfo.pNext = nullptr;
        // commandBufferAllocateInfo.commandPool = s_frameContext[i].commandPool;
        // commandBufferAllocateInfo.commandBufferCount = 1;
        // commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        // TK_ASSERT_VK_RESULT(
        //     vkAllocateCommandBuffers(context.device, &commandBufferAllocateInfo, &s_frameContext[i].commandBuffer),
        //     "Could not allocate command buffers");
    }
}

static void initResources() {
    createNewGeometryVertexBuffer();
    createNewGeometryIndexBuffer();
}

static bool checkDeviceExtensionSupport() {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(context, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(context, nullptr, &extensionCount, deviceExtensions.data());

    uint32_t requiredExtentionCount = requiredExtensions.size();

    for (const auto& req : requiredExtensions) {
        std::string requiredExtension(req);
        for (const auto& deviceExtension : deviceExtensions) {
            if (std::string(deviceExtension.extensionName) == requiredExtension) {
                --requiredExtentionCount;
                break;
            }
        }
    }

    return requiredExtentionCount == 0;
}

static bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    for (const char* requiredLayer : validationLayers) {
        bool layerFound = false;

        for (const auto& foundLayer : layers) {
            if (strcmp(requiredLayer, foundLayer.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) return false;
    }

    return true;
}

static VkSurfaceFormatKHR findSurfaceFormat() {
    for (const auto& format : context.physicalDeviceData.surfaceFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return context.physicalDeviceData.surfaceFormats[0];
}

static VkPresentModeKHR findPresentMode() {
    for (const auto& presentMode : context.physicalDeviceData.presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkImageAspectFlags getImageAspectFlags(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_SRGB:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            std::unreachable();
    }
}

static VkFormat mapFormat(ColorFormat format) {
    switch (format) {
        case ColorFormat::R8:
            return VK_FORMAT_R8_SRGB;
        case ColorFormat::RGBA8:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case ColorFormat::Depth:
            return VK_FORMAT_D32_SFLOAT;
        case ColorFormat::Stencil:
            return VK_FORMAT_S8_UINT;
        case ColorFormat::DepthStencil:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        default:
            std::unreachable();
    }
}

std::optional<uint32_t> acquireNextImage(FrameContext& context);

}  // namespace Toki
