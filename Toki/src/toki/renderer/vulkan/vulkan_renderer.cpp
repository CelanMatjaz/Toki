#include "vulkan_renderer.h"

#include "tkpch.h"
#include "toki/core/application.h"
#include "vulkan_constants.h"
#include "vulkan_pipeline.h"

namespace Toki {

    VulkanRenderer::VulkanRenderer() {
        init();
    }

    VulkanRenderer::~VulkanRenderer() {
        cleanup();
    }

    void VulkanRenderer::init() {
        createInstance();
        createSurface();
        physicalDevice = VulkanDevice::enumeratePhysicalDevices().front();
        VulkanDevice::initQueueFamilyIndexes();
        VulkanDevice::initQueueFamilyProperties();
        device = VulkanDevice::createDevice();
        vkGetDeviceQueue(device, VulkanDevice::getQueueFamilyIndexes().graphicsQueueIndex, 0, &graphicsQueue);
        createCommandPool();
        createSwapchain();
        createFrames();
        createDepthBuffer();
        createRenderPass();
        createFrameBuffers();
    }

    void VulkanRenderer::cleanup() {
        vkDeviceWaitIdle(device);

        cleanupSwapchain();

        for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
            frames[i].cleanup();
        }

        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    void VulkanRenderer::recreateSwapchain() {
        vkDeviceWaitIdle(device);

        cleanupSwapchain();
        createSwapchain();
        createRenderPass();
        createDepthBuffer();
        createFrameBuffers();
        VulkanPipeline::recreatePipelines();
    }

    void VulkanRenderer::cleanupSwapchain() {
        depthBuffer.cleanup();
        vkDestroyRenderPass(device, renderPass, nullptr);

        for (uint32_t i = 0; i < frameBuffers.size(); ++i) {
            vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
        }

        for (auto& imageView : swapchainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    void VulkanRenderer::beginFrame() {
        FrameData* frame = getCurrentFrame();
        uint64_t timeout = UINT64_MAX - 1;
        VkResult result = vkAcquireNextImageKHR(device, swapchain, timeout, frame->presentSemaphore, nullptr, &imageIndex);

        TK_ASSERT(vkWaitForFences(device, 1, &frame->renderFence, VK_TRUE, timeout) == VK_SUCCESS);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapchain();
            // return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swapchain image");
        }

        TK_ASSERT(vkResetFences(device, 1, &frame->renderFence) == VK_SUCCESS);
        TK_ASSERT((result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) && "Failed to acquire swapchain image");

        isFrameStarted = true;

        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        TK_ASSERT(vkBeginCommandBuffer(frame->commandBuffer, &commandBufferBeginInfo) == VK_SUCCESS);

        VkClearValue clearValue{};
        clearValue.color = { 0.1f, 0.1f, 0.1f, 1.0f };
        VkClearValue depthClear;
        depthClear.depthStencil.depth = 1.f;

        std::array<VkClearValue, 2> clearValues = { clearValue, depthClear };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = swapchainExtent;
        renderPassBeginInfo.framebuffer = frameBuffers[imageIndex];
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(frame->commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        currentCommandBuffer = frame->commandBuffer;
    }

    void VulkanRenderer::endFrame() {
        if (!isFrameStarted) return;

        FrameData* frame = getCurrentFrame();
        vkCmdEndRenderPass(frame->commandBuffer);

        TK_ASSERT(isFrameStarted && "Can't call endFrame while frame is not in progress");
        vkEndCommandBuffer(frame->commandBuffer);

        VkSemaphore waitSemaphores[] = { frame->presentSemaphore };
        VkSemaphore signalSemaphores[] = { frame->renderSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame->commandBuffer;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        TK_ASSERT(vkResetFences(device, 1, &frame->renderFence) == VK_SUCCESS);
        TK_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, frame->renderFence) == VK_SUCCESS);

        VkSwapchainKHR swapchains[] = { swapchain };
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains = swapchains;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &frame->renderSemaphore;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &this->imageIndex;

        VkResult result = vkQueuePresentKHR(graphicsQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Application::getWindow()->wasResized()) {
            Application::getWindow()->resetResizedFlag();
            recreateSwapchain();
        }
        else if (result != VK_SUCCESS) {
            TK_ASSERT(false && "Failed to present swapchain image");
        }

        isFrameStarted = false;
        ++currentFrame;
    }

    void VulkanRenderer::createInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.engineVersion = ENGINE_VERSION;
        appInfo.pEngineName = ENGINE_NAME;
        appInfo.pApplicationName = ENGINE_NAME;
        appInfo.applicationVersion = ENGINE_VERSION;
        appInfo.apiVersion = VK_API_VERSION_1_3;

        const char** glfwExtensions;
        uint32_t extensionCount = 0;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        auto result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledExtensionCount = extensionCount;
        instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;

#ifndef DIST
        TK_ASSERT(VulkanDevice::checkForValidationLayerSupport());
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif

        TK_ASSERT(vkCreateInstance(&instanceCreateInfo, nullptr, &instance) == VK_SUCCESS);
    }

    void VulkanRenderer::createSurface() {
        TK_ASSERT(glfwCreateWindowSurface(instance, Application::getNativeWindow(), nullptr, &VulkanRenderer::surface) == VK_SUCCESS);
    }

    void VulkanRenderer::createCommandPool() {
        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = VulkanDevice::getQueueFamilyIndexes().graphicsQueueIndex;

        TK_ASSERT(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool) == VK_SUCCESS);
    }

    void VulkanRenderer::createSwapchain() {
        std::vector<VkQueueFamilyProperties> queueFamilyProperties = VulkanDevice::getQueueFamilyProperties();
        const auto [graphicsQueueIndex, presentQueueIndex] = VulkanDevice::getQueueFamilyIndexes();
        const auto [windowWidth, windowHeight] = Application::getWindow()->getWindowDimensions();

        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, formats.data());

        swapchainImageFormat = VulkanDevice::findSurfaceFormat(formats).format;

        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

        swapchainExtent = VulkanDevice::getExtent(surfaceCapabilities);

        VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR; // TODO: use this for FPS limits later
        VkSurfaceTransformFlagBitsKHR preTransform = VulkanDevice::getPreTransform(surfaceCapabilities);
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VulkanDevice::getCompositeAlpha(surfaceCapabilities);
        uint32_t imageCount = VulkanDevice::getImageCount(surfaceCapabilities);

        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = swapchainImageFormat;
        swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainCreateInfo.imageExtent = swapchainExtent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.compositeAlpha = compositeAlpha;
        swapchainCreateInfo.presentMode = swapchainPresentMode;
        swapchainCreateInfo.clipped = true;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.preTransform = preTransform;
        swapchainCreateInfo.queueFamilyIndexCount = 0;

        uint32_t queueFamilyIndecies[2] = { graphicsQueueIndex, presentQueueIndex };

        if (queueFamilyIndecies[0] != queueFamilyIndecies[1]) {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount = 2;
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndecies;
        }

        TK_ASSERT(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) == VK_SUCCESS);

        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
        swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

        swapchainImageViews.resize(swapchainImages.size());

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.format = swapchainImageFormat;
        imageViewCreateInfo.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        };
        imageViewCreateInfo.subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
        };
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.flags = 0;

        for (uint32_t i = 0; i < surfaceCapabilities.minImageCount; ++i) {
            imageViewCreateInfo.image = swapchainImages[i];
            TK_ASSERT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]) == VK_SUCCESS);
        }
    }

    void VulkanRenderer::createRenderPass() {
        const auto [graphicsQueueIndex, presentQueueIndex] = VulkanDevice::getQueueFamilyIndexes();

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapchainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference{};
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorReference.attachment = 0;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentRef.attachment = 1;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency colorDependency{};
        colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        colorDependency.dstSubpass = 0;
        colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        colorDependency.srcAccessMask = 0;
        colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDependency depthDependency{};
        depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        depthDependency.dstSubpass = 0;
        depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDependency.srcAccessMask = 0;
        depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        std::array<VkSubpassDependency, 2> dependencies = { colorDependency, depthDependency };
        std::array<VkSubpassDescription, 1> subpasses = { subpass };

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = dependencies.size();
        renderPassCreateInfo.pDependencies = dependencies.data();

        TK_ASSERT(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) == VK_SUCCESS);
    }

    void VulkanRenderer::createFrames() {
        const auto [graphicsQueueIndex, presentQueueIndex] = VulkanDevice::getQueueFamilyIndexes();

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
            VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.commandPool = commandPool;
            commandBufferAllocateInfo.commandBufferCount = 1;
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            TK_ASSERT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &frames[i].commandBuffer) == VK_SUCCESS);
            TK_ASSERT(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence) == VK_SUCCESS);
            TK_ASSERT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore) == VK_SUCCESS);
            TK_ASSERT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore) == VK_SUCCESS);
        }
    }

    void VulkanRenderer::createFrameBuffers() {
        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.width = swapchainExtent.width;
        framebufferCreateInfo.height = swapchainExtent.height;
        framebufferCreateInfo.layers = 1;

        frameBuffers.resize(swapchainImageViews.size());

        for (uint32_t i = 0; i < swapchainImageViews.size(); ++i) {
            std::vector<VkImageView> attachments = {
                swapchainImageViews[i],
                depthBuffer.imageView
            };

            framebufferCreateInfo.pAttachments = attachments.data();
            framebufferCreateInfo.attachmentCount = attachments.size();

            TK_ASSERT(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &frameBuffers[i]) == VK_SUCCESS);
        }
    }

    void VulkanRenderer::createDepthBuffer() {
        const auto [graphicsQueueIndex, presentQueueIndex] = VulkanDevice::getQueueFamilyIndexes();
        const auto [windowWidth, windowHeight] = Application::getWindow()->getWindowDimensions();

        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

        depthFormat = VulkanDevice::findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );

        VkExtent2D extent = VulkanDevice::getExtent(surfaceCapabilities);
        VkImageTiling tiling = VulkanDevice::findTiling(depthFormat);

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent = { extent.width, extent.height, 1 };
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = depthFormat;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateImage(device, &imageCreateInfo, nullptr, &depthBuffer.image);

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(device, depthBuffer.image, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = VulkanDevice::findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        TK_ASSERT(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &depthBuffer.memory) == VK_SUCCESS);
        TK_ASSERT(vkBindImageMemory(device, depthBuffer.image, depthBuffer.memory, 0) == VK_SUCCESS);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.format = depthFormat;
        imageViewCreateInfo.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        };
        imageViewCreateInfo.subresourceRange = {
            VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1
        };
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.image = depthBuffer.image;

        TK_ASSERT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &depthBuffer.imageView) == VK_SUCCESS);
    }

    VkCommandBuffer VulkanRenderer::startCommandBuffer() {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        TK_ASSERT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer) == VK_SUCCESS);

        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        TK_ASSERT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) == VK_SUCCESS);

        return commandBuffer;
    }

    void VulkanRenderer::endCommandBuffer(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

}