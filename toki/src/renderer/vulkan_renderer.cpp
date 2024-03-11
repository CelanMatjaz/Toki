#include "vulkan_renderer.h"

#include <print>
#include <vector>

#include "platform.h"
#include "renderer/vulkan_buffer.h"
#include "renderer/vulkan_graphics_pipeline.h"
#include "renderer/vulkan_image.h"
#include "renderer/vulkan_render_pass.h"
#include "renderer/vulkan_utils.h"
#include "toki/core/assert.h"
#include "toki/events/event.h"
#include "vulkan/vulkan_core.h"

#ifdef TK_WINDOW_SYSTEM_GLFW
#include "GLFW/glfw3.h"
#endif
#include "vulkan_rendering_context.h"

namespace Toki {

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};
const std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                                      /* VK_NV_FILL_RECTANGLE_EXTENSION_NAME */ };

bool checkDeviceExtensionSupport(Ref<VulkanContext> m_context);
bool checkValidationLayerSupport();

VulkanRenderer::VulkanRenderer() : m_context(createRef<VulkanContext>()) {
    Event::bindEvent(EventType::WindowResize, this, [this](void* sender, void* receiver, const Event& event) { m_wasWindowResized = true; });
}

void VulkanRenderer::init() {
    createInstance();

    // Select physical device
    {
        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(m_context->instance, &physicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(m_context->instance, &physicalDeviceCount, physicalDevices.data());
        m_context->physicalDevice = physicalDevices[0];
    }

    // Create device
    {
        WindowConfig windowConfig{};
        windowConfig.showOnCreate = false;
        Ref<Window> tempWindow = Window::create(windowConfig);
        VkSurfaceKHR tempSurface = VulkanUtils::createSurface(m_context, tempWindow);
        createDevice(tempSurface);
        vkDestroySurfaceKHR(m_context->instance, tempSurface, m_context->allocationCallbacks);
    }

    initCommandPools();
    initFrames();
    initDescriptorPools();

    VulkanBuffer::s_context = m_context;
    VulkanImage::s_context = m_context;
    VulkanGraphicsPipeline::s_context = m_context;

    m_context->submitSingleUseCommands = [this](std::function<void(VkCommandBuffer)> fn) {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = m_extraCommandPools[0];
        commandBufferAllocateInfo.commandBufferCount = 1;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkCommandBuffer commandBuffer;
        TK_ASSERT_VK_RESULT(
            vkAllocateCommandBuffers(m_context->device, &commandBufferAllocateInfo, &commandBuffer), "Could not allocate single use command buffer");

        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        TK_ASSERT_VK_RESULT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), "Could not begin recording single use command buffer");

        fn(commandBuffer);

        TK_ASSERT_VK_RESULT(vkEndCommandBuffer(commandBuffer), "Could not end recording single use commdn buffer");

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        TK_ASSERT_VK_RESULT(vkQueueSubmit(m_context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE), "");
        TK_ASSERT_VK_RESULT(vkQueueWaitIdle(m_context->graphicsQueue), "");
        vkFreeCommandBuffers(m_context->device, m_extraCommandPools[0], 1, &commandBuffer);
    };
}

void VulkanRenderer::shutdown() {
    vkDeviceWaitIdle(m_context->device);

    m_swapchains.clear();

    destroyDescriptorPools();
    destroyCommandPools();
    destroyFrames();

    vkDestroyDevice(m_context->device, m_context->allocationCallbacks);
    vkDestroyInstance(m_context->instance, m_context->allocationCallbacks);
}

void VulkanRenderer::beginFrame() {
    FrameData& frame = m_frameData[m_currentFrame];

    TK_ASSERT(m_swapchains.size() == 1, "Multiple swapchains are not yet supported");

    auto swapchain = m_swapchains[0];

    if (m_wasWindowResized) {
        m_swapchains[0]->recreate();
        m_wasWindowResized = false;
    }

    std::optional<uint32_t> imageIndex = swapchain->acquireNextImage(frame);
    if (!imageIndex.has_value()) {
        return;
    }

    vkResetFences(m_context->device, 1, &frame.renderFence);

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    TK_ASSERT_VK_RESULT(vkBeginCommandBuffer(frame.commandBuffer, &commandBufferBeginInfo), "Error starting command buffer recording");

    for (uint32_t i = 0; i < m_swapchains.size(); ++i) {
        m_swapchains[i]->transitionLayout(frame.commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
}

void VulkanRenderer::endFrame() {
    FrameData& frame = m_frameData[m_currentFrame];

    // TODO: resize on demand
    static std::vector<VkSwapchainKHR> swapchainHandles(m_swapchains.size());
    static std::vector<uint32_t> swapchainIndices(m_swapchains.size());

    for (uint32_t i = 0; i < m_swapchains.size(); ++i) {
        swapchainHandles[i] = m_swapchains[i]->getSwapchainHandle();
        swapchainIndices[i] = m_swapchains[i]->getCurrentImageIndex();
        m_swapchains[i]->transitionLayout(frame.commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    vkEndCommandBuffer(frame.commandBuffer);

    VkSemaphore waitSemaphores[] = { frame.presentSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { frame.presentSemaphore };

    std::vector<VkCommandBuffer> commandBuffers = { frame.commandBuffer };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    TK_ASSERT_VK_RESULT(vkQueueSubmit(m_context->graphicsQueue, 1, &submitInfo, frame.renderFence), "Could not submit");

    std::vector<VkResult> results(m_swapchains.size());

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = swapchainHandles.size();
    presentInfo.pSwapchains = swapchainHandles.data();
    presentInfo.pImageIndices = swapchainIndices.data();
    presentInfo.pResults = results.data();

    VkResult result = vkQueuePresentKHR(m_context->presentQueue, &presentInfo);

    uint32_t i = 0;
    for (auto& result : results) {
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_wasWindowResized) {
            m_swapchains[i]->recreate();
            m_currentFrame = m_swapchains[i]->getCurrentImageIndex();
            m_wasWindowResized = false;
            return;
        } else {
            TK_ASSERT_VK_RESULT(result, "Failed to present swapchain image");
        }
        ++i;
    }

    m_currentFrame = (m_swapchains[0]->getCurrentImageIndex() + 1) % MAX_FRAMES;

    VkResult waitFencesResult = vkWaitForFences(m_context->device, 1, &frame.renderFence, VK_TRUE, UINT64_MAX);
    TK_ASSERT(waitFencesResult == VK_SUCCESS || waitFencesResult == VK_TIMEOUT, "Failed waiting for fences");
}

void VulkanRenderer::submit(Ref<RenderPass> rp, RendererSubmitFn submitFn) {
    VulkanRenderPass* renderPass = (VulkanRenderPass*) rp.get();

    VulkanRenderingContext ctx{ m_frameData[m_currentFrame].commandBuffer };

    auto [width, height] = m_swapchains[0]->getExtent();

    VkViewport viewport{};
    viewport.width = width;
    viewport.height = height;
    viewport.x = 0;
    viewport.y = 0;
    vkCmdSetViewport(m_frameData[m_currentFrame].commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width = width;
    scissor.extent.height = height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(m_frameData[m_currentFrame].commandBuffer, 0, 1, &scissor);

    vkCmdSetLineWidth(m_frameData[m_currentFrame].commandBuffer, 1.0f);

    renderPass->begin(ctx, m_swapchains[0]->getExtent(), m_swapchains[0]->getCurrentImageView());
    submitFn(ctx);
    renderPass->end(ctx);
}

void VulkanRenderer::waitForDevice() {
    vkDeviceWaitIdle(m_context->device);
}

void VulkanRenderer::createSwapchain(Ref<Window> window) {
    m_swapchains.emplace_back(VulkanSwapchain::create(m_context, {}, window));
}

void VulkanRenderer::createInstance() {
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

    TK_ASSERT_VK_RESULT(vkCreateInstance(&instanceCreateInfo, m_context->allocationCallbacks, &m_context->instance), "Could not create instance");
}

void VulkanRenderer::createDevice(VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_context->physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_context->physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_context->graphicsFamilyIndex = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_context->physicalDevice, i, surface, &presentSupport);

        if (presentSupport) {
            m_context->presentFamilyIndex = i;
        }

        if (m_context->graphicsFamilyIndex >= 0 && m_context->presentFamilyIndex >= 0) break;
    }

    if (m_context->graphicsFamilyIndex < 0 || m_context->presentFamilyIndex < 0) {
        std::println("Device does not support graphics");
        exit(1);
    }

    auto isDeviceSuitable = [this, surface](Ref<VulkanContext> m_context) {
        VkPhysicalDevice physicalDevice = m_context->physicalDevice;

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(m_context->physicalDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(m_context->physicalDevice, &deviceFeatures);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        if (formatCount) {
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        if (presentModeCount) {
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        }

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && checkDeviceExtensionSupport(m_context) && formatCount &&
               presentModeCount;
    };

    if (!isDeviceSuitable(m_context)) {
        std::println("Device missing required features and is not suitable");
        exit(1);
    }

    float queuePriority = 1.0f;

    uint32_t queueFamilies[2] = { (uint32_t) m_context->graphicsFamilyIndex, (uint32_t) m_context->presentFamilyIndex };
    VkDeviceQueueCreateInfo deviceQueueCreateInfos[2] = {};

    for (uint32_t i = 0; i < 2; ++i) {
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
    deviceCreateInfo.queueCreateInfoCount = m_context->graphicsFamilyIndex == m_context->presentFamilyIndex ? 1 : 2;
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    deviceCreateInfo.enabledExtensionCount = requiredExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifndef TK_DIST
    if (checkValidationLayerSupport()) {
        deviceCreateInfo.enabledLayerCount = validationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        TK_ASSERT(false, "1 or more validation layers not supported by the system");
    }
#endif  // !DIST

    TK_ASSERT_VK_RESULT(
        vkCreateDevice(m_context->physicalDevice, &deviceCreateInfo, m_context->allocationCallbacks, &m_context->device), "Could not create device");

    vkGetDeviceQueue(m_context->device, m_context->graphicsFamilyIndex, 0, &m_context->graphicsQueue);
    vkGetDeviceQueue(m_context->device, m_context->presentFamilyIndex, 0, &m_context->presentQueue);
}

void VulkanRenderer::initFrames() {
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = m_context->graphicsFamilyIndex;

    uint32_t commandBufferCount = 1;

    VkCommandBufferAllocateInfo commandBufferALlocateInfo{};
    commandBufferALlocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferALlocateInfo.commandBufferCount = commandBufferCount;
    commandBufferALlocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
        TK_ASSERT_VK_RESULT(
            vkCreateFence(m_context->device, &fenceCreateInfo, m_context->allocationCallbacks, &m_frameData[i].renderFence),
            "Could not create render fence");
        TK_ASSERT_VK_RESULT(
            vkCreateSemaphore(m_context->device, &semaphoreCreateInfo, m_context->allocationCallbacks, &m_frameData[i].renderSemaphore),
            "Could not create render semaphore");
        TK_ASSERT_VK_RESULT(
            vkCreateSemaphore(m_context->device, &semaphoreCreateInfo, m_context->allocationCallbacks, &m_frameData[i].presentSemaphore),
            "Could not create present semaphore");

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = m_commandPools[0][i];
        commandBufferAllocateInfo.commandBufferCount = 1;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        TK_ASSERT_VK_RESULT(vkAllocateCommandBuffers(m_context->device, &commandBufferAllocateInfo, &m_frameData[i].commandBuffer), "");
    }
}

void VulkanRenderer::initCommandPools() {
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = m_context->graphicsFamilyIndex;

    TK_ASSERT(s_renderThreadCount == 1, "More than 1 render thread no supported");  // TODO: remove temp code
    m_commandPools.resize(s_renderThreadCount);
    for (uint32_t frameIndex = 0; frameIndex < MAX_FRAMES; ++frameIndex) {
        for (uint32_t i = 0; i < s_renderThreadCount; ++i) {
            TK_ASSERT_VK_RESULT(
                vkCreateCommandPool(m_context->device, &commandPoolCreateInfo, m_context->allocationCallbacks, &m_commandPools[i][frameIndex]),
                "Could not create command pool");
        }
    }

    m_extraCommandPools.resize(s_extraCommandPoolCount);
    for (uint32_t i = 0; i < s_extraCommandPoolCount; ++i) {
        TK_ASSERT_VK_RESULT(
            vkCreateCommandPool(m_context->device, &commandPoolCreateInfo, m_context->allocationCallbacks, &m_extraCommandPools[i]),
            "Could not create frame command pool");
    }
}

void VulkanRenderer::initDescriptorPools() {
    static constexpr uint32_t MAX_SETS = 100;

    std::vector<VkDescriptorPoolSize> poolSizes = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
                                                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
                                                    { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
                                                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 } };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = MAX_SETS;
    descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

    TK_ASSERT_VK_RESULT(
        vkCreateDescriptorPool(m_context->device, &descriptorPoolCreateInfo, m_context->allocationCallbacks, &m_context->descriptorPool),
        "Could not create ddescriptor pool");
}

void VulkanRenderer::destroyFrames() {
    for (const auto& frameData : m_frameData) {
        vkDestroyFence(m_context->device, frameData.renderFence, m_context->allocationCallbacks);
        vkDestroySemaphore(m_context->device, frameData.presentSemaphore, m_context->allocationCallbacks);
        vkDestroySemaphore(m_context->device, frameData.renderSemaphore, m_context->allocationCallbacks);
    }
}

void VulkanRenderer::destroyCommandPools() {
    for (auto& commandPools : m_commandPools) {
        for (auto& pool : commandPools) {
            vkDestroyCommandPool(m_context->device, pool, m_context->allocationCallbacks);
        }
    }

    for (auto& pool : m_extraCommandPools) {
        vkDestroyCommandPool(m_context->device, pool, m_context->allocationCallbacks);
    }
}

void VulkanRenderer::destroyDescriptorPools() {
    vkDestroyDescriptorPool(m_context->device, m_context->descriptorPool, m_context->allocationCallbacks);
}

#pragma region Utils

bool checkDeviceExtensionSupport(Ref<VulkanContext> m_context) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(m_context->physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(m_context->physicalDevice, nullptr, &extensionCount, deviceExtensions.data());

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

bool checkValidationLayerSupport() {
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

#pragma endregion Utils

}  // namespace Toki
