#include "vulkan_renderer.h"

#include <cstdint>
#include <mutex>

#include "GLFW/glfw3.h"
#include "core/assert.h"
#include "core/core.h"
#include "core/window.h"
#include "events/events.h"
#include "iostream"
#include "string"
#include "vector"
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_framebuffer.h"
#include "vulkan/vulkan_pipelines/vulkan_graphics_pipeline.h"
#include "vulkan/vulkan_render_pass.h"
#include "vulkan/vulkan_swapchain.h"
#include "vulkan/vulkan_types.h"

namespace Toki {

const std::vector<const char*> VulkanRenderer::validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};
const std::vector<const char*> VulkanRenderer::requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

VulkanRenderer::VulkanRenderer(Ref<Window> window) : Renderer(window) {
    createInstance();

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(context.instance, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(context.instance, &physicalDeviceCount, physicalDevices.data());

    context.physicalDevice = physicalDevices[0];
}

VulkanRenderer::~VulkanRenderer() {}

static Ref<VulkanRenderPass> renderPass;
static Ref<GraphicsPipeline> graphicsPipeline;
static bool wasResized = false;

void VulkanRenderer::init() {
    SwapchainConfig swapchainConfig{};
    Ref<VulkanSwapchain> mainSwapchain = createRef<VulkanSwapchain>(&context, swapchainConfig, window);
    swapchains.emplace_back(mainSwapchain);

    // Need window created to check for surface support and get present queue
    createDevice(mainSwapchain->getSurface());

    initFrames();
    mainSwapchain->init();

    RenderPassAttachment attachment1{};
    attachment1.format = Format::RGBA;
    attachment1.type = AttachmentType::Color;
    attachment1.sampleCount = SampleCount::SampleCount1;
    attachment1.storeOp = AttachmentStoreOp::Store;
    attachment1.loadOp = AttachmentLoadOp::Clear;

    // RenderPassAttachment attachment2 = attachment1;
    // attachment2.format = Format::DepthStencil;
    // attachment2.type = AttachmentType::DepthStencil;

    RenderPassConfig renderPassConfig{};
    renderPassConfig.subpassCount = 1;
    renderPassConfig.attachments = { attachment1 /* , attachment2 */ };
    renderPass = createRef<VulkanRenderPass>(&context, renderPassConfig);

    VulkanGraphicsPipelineConfig gpConfig{};
    gpConfig.shaderPath = "assets/shaders/test_shader.glsl";
    gpConfig.renderPass = renderPass->getHandle();
    gpConfig.subpass = 0;
    // gpConfig.bindingDescriptions = {{0, 1, VK_VERTEX_INPUT_RATE_VERTEX}};
    // gpConfig.attributeDescriptions = {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}};
    graphicsPipeline = createRef<GraphicsPipeline>(&context, gpConfig);

    FramebufferConfig fbConfig{};
    fbConfig.renderPass = renderPass->getHandle();

    for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
        frameData[i].framebuffers.emplace_back(mainSwapchain->createFramebuffer(fbConfig, i));
    }
}

void VulkanRenderer::shutdown() {
    vkDeviceWaitIdle(context.device);

    graphicsPipeline.reset();
    renderPass.reset();

    for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
        FrameData& frame = frameData[i];
        frame.framebuffers.clear();
        vkDestroyCommandPool(context.device, frame.commandPool, context.allocationCallbacks);
        vkDestroyFence(context.device, frame.renderFence, context.allocationCallbacks);
        vkDestroySemaphore(context.device, frame.presentSemaphore, context.allocationCallbacks);
        vkDestroySemaphore(context.device, frame.renderSemaphore, context.allocationCallbacks);
    }

    swapchains.clear();

    vkDestroyDevice(context.device, context.allocationCallbacks);
    vkDestroyInstance(context.instance, context.allocationCallbacks);
}

bool VulkanRenderer::beginFrame() {
    std::scoped_lock lck(rendererLock);

    vkDeviceWaitIdle(context.device);

    FrameData& frame = frameData[currentFrame];

    Ref<VulkanSwapchain> mainSwapchain = swapchains[0];

    uint32_t imageIndex = mainSwapchain->acquireNextImage(frame);
    if (imageIndex == MAX_FRAMES) {
        std::scoped_lock lck(presentQueueLock);
        currentFrame = imageIndex;
        swapchains[0]->recreate();
        return false;
    }

    vkResetFences(context.device, 1, &frame.renderFence);

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(frame.commandBuffers[0], &commandBufferBeginInfo);

    VkClearValue clearValue{};
    static float t = 0.0f;
    clearValue.color = { 0.59f, 0.59f, t, 1.0f };
    t = t + 0.01f;
    if (t > 1.0f) t = 0.0f;

    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = renderPass->getHandle();
    beginInfo.framebuffer = frame.framebuffers[0]->getHandle();
    beginInfo.renderArea.extent = frame.framebuffers[0]->getExtent();
    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(frame.commandBuffers[0], &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(frame.commandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->getHandle());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = mainSwapchain->getExtent().width;
    viewport.height = mainSwapchain->getExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(frame.commandBuffers[0], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = frame.framebuffers[0]->getExtent();
    vkCmdSetScissor(frame.commandBuffers[0], 0, 1, &scissor);

    vkCmdDraw(frame.commandBuffers[0], 3, 1, 0, 0);

    vkCmdEndRenderPass(frame.commandBuffers[0]);

    return true;
}

void VulkanRenderer::endFrame() {
    std::scoped_lock lck(rendererLock);

    FrameData& frame = frameData[currentFrame];

    vkEndCommandBuffer(frame.commandBuffers[0]);

    VkSemaphore waitSemaphores[] = { frame.presentSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { frame.presentSemaphore };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = frame.commandBuffers.size();
    submitInfo.pCommandBuffers = frame.commandBuffers.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    graphicsQueueLock.lock();
    TK_ASSERT_VK_RESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, frame.renderFence), "Could not submit");
    graphicsQueueLock.unlock();

    static std::vector<VkSwapchainKHR> swapchainHandles(swapchains.size());
    static std::vector<uint32_t> swapchainIndices(swapchains.size());
    for (uint32_t i = 0; i < swapchains.size(); ++i) {
        swapchainHandles[i] = swapchains[i]->getSwapchain();
        swapchainIndices[i] = swapchains[i]->getImageIndex();
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = swapchainHandles.size();
    presentInfo.pSwapchains = swapchainHandles.data();
    presentInfo.pImageIndices = swapchainIndices.data();
    presentInfo.pResults = nullptr;

    presentQueueLock.lock();
    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    presentQueueLock.unlock();

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || wasResized) {
        wasResized = false;
        std::scoped_lock lck(presentQueueLock);
        swapchains[0]->recreate();
        currentFrame = swapchains[0]->getImageIndex();
        return;
    } else
        TK_ASSERT_VK_RESULT(result, "Failed to present swapchain image");

    currentFrame = (swapchains[0]->getImageIndex() + 1) % MAX_FRAMES;
}

void VulkanRenderer::onEvent(Event& event) {
    std::scoped_lock lck(rendererLock);
    std::scoped_lock gqlck(graphicsQueueLock);
    std::scoped_lock pqlck(presentQueueLock);

    vkDeviceWaitIdle(context.device);

    switch (event.getType()) {
        case Toki::EventType::WindowResize:
            if (event.getData(0) == 0 || event.getData(1) == 0) break;
            wasResized = true;
            break;
        default:;
    }
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
    commandPoolCreateInfo.queueFamilyIndex = context.graphicsFamilyIndex;

    uint32_t commandBufferCount = 1;

    VkCommandBufferAllocateInfo commandBufferALlocateInfo{};
    commandBufferALlocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferALlocateInfo.commandBufferCount = commandBufferCount;
    commandBufferALlocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
        TK_ASSERT_VK_RESULT(
            vkCreateCommandPool(context.device, &commandPoolCreateInfo, context.allocationCallbacks, &frameData[i].commandPool),
            "Could not create frame command pool"
        );

        commandBufferALlocateInfo.commandPool = frameData[i].commandPool;

        frameData[i].commandBuffers.resize(commandBufferALlocateInfo.commandBufferCount);
        TK_ASSERT_VK_RESULT(
            vkAllocateCommandBuffers(context.device, &commandBufferALlocateInfo, frameData[i].commandBuffers.data()),
            "Could not allocate command buffer(s)"
        );
        TK_ASSERT_VK_RESULT(
            vkCreateFence(context.device, &fenceCreateInfo, context.allocationCallbacks, &frameData[i].renderFence), "Could not create render fence"
        );
        TK_ASSERT_VK_RESULT(
            vkCreateSemaphore(context.device, &semaphoreCreateInfo, context.allocationCallbacks, &frameData[i].renderSemaphore),
            "Could not create render semaphore"
        );
        TK_ASSERT_VK_RESULT(
            vkCreateSemaphore(context.device, &semaphoreCreateInfo, context.allocationCallbacks, &frameData[i].presentSemaphore),
            "Could not create present semaphore"
        );
    }
}

void VulkanRenderer::createInstance() {
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Toki";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "Toki";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    const char* extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef WINDOW_SYSTEM_NATIVE
                                 VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif  // WINDOW_SYSTEM_NATIVE
    };

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = sizeof(extensions) / sizeof(*extensions);
    instanceCreateInfo.ppEnabledExtensionNames = extensions;

#ifndef DIST
    if (checkValidationLayerSupport()) {
        instanceCreateInfo.enabledLayerCount = validationLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        std::cout << "1 or more validation layers not supported by the system\n";
    }
#endif  // !DIST

    TK_ASSERT_VK_RESULT(vkCreateInstance(&instanceCreateInfo, context.allocationCallbacks, &context.instance), "Could not create instance");

    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    }
}

void VulkanRenderer::createDevice(VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            context.graphicsFamilyIndex = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(context.physicalDevice, i, surface, &presentSupport);

        if (presentSupport) {
            context.presentFamilyIndex = i;
        }

        if (context.graphicsFamilyIndex >= 0 && context.presentFamilyIndex >= 0) break;
    }

    if (context.graphicsFamilyIndex < 0 || context.presentFamilyIndex < 0) {
        std::cout << "Device does not support graphics\n";
        exit(1);
    }

    auto isDeviceSuitable = [this, surface]() {
        VkPhysicalDevice physicalDevice = this->context.physicalDevice;

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(context.physicalDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(context.physicalDevice, &deviceFeatures);

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

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && checkDeviceExtensionSupport() && formatCount &&
               presentModeCount;
    };

    if (!isDeviceSuitable()) {
        std::cout << "Device missing required features and is not suitable\n";
        exit(1);
    }

    float queuePriority = 1.0f;

    uint32_t queueFamilies[2] = { (uint32_t) context.graphicsFamilyIndex, (uint32_t) context.presentFamilyIndex };
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

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
    deviceCreateInfo.queueCreateInfoCount = context.graphicsFamilyIndex == context.presentFamilyIndex ? 1 : 2;
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    deviceCreateInfo.enabledExtensionCount = requiredExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifndef DIST
    if (checkValidationLayerSupport()) {
        deviceCreateInfo.enabledLayerCount = validationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        std::cout << "1 or more validation layers not supported by the system\n";
    }
#endif  // !DIST

    TK_ASSERT_VK_RESULT(
        vkCreateDevice(context.physicalDevice, &deviceCreateInfo, context.allocationCallbacks, &context.device), "Could not create device"
    );

    vkGetDeviceQueue(context.device, context.graphicsFamilyIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(context.device, context.presentFamilyIndex, 0, &presentQueue);
}

bool VulkanRenderer::checkValidationLayerSupport() {
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

bool VulkanRenderer::checkDeviceExtensionSupport() {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(context.physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(context.physicalDevice, nullptr, &extensionCount, deviceExtensions.data());

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

}  // namespace Toki
