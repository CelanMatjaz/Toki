#include "vulkan_renderer.h"

#include "tkpch.h"
#include "toki/core/application.h"
#include "vulkan_constants.h"

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
        physicalDevice = instance.enumeratePhysicalDevices().front();
        VulkanDevice::initQueueFamilyIndexes();
        device = VulkanDevice::createDevice();
        createCommandPool();
        createSwapchain();
        createFrames();
        createDepthBuffer();
        createRenderPass();
    }

    void VulkanRenderer::cleanup() {
        device.destroyRenderPass(renderPass);
        depthBuffer.cleanup();

        for (auto& imageView : swapchainImageViews) {
            device.destroyImageView(imageView);
        }

        device.destroySwapchainKHR(swapchain);

        for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
            frames[i].cleanup();
        }

        device.destroyCommandPool(commandPool);
        device.destroy();
        vkDestroySurfaceKHR(instance, static_cast<VkSurfaceKHR>(VulkanRenderer::surface), nullptr);
        instance.destroy();
    }

    void VulkanRenderer::createInstance() {
        vk::ApplicationInfo appInfo {ENGINE_NAME, ENGINE_VERSION, ENGINE_NAME, ENGINE_VERSION, VK_API_VERSION_1_3};

        const char** glfwExtensions;
        uint32_t extensionCount = 0;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        std::vector<vk::ExtensionProperties> properties = vk::enumerateInstanceExtensionProperties();

        vk::InstanceCreateInfo instanceCreateInfo{ {}, & appInfo, {}, {}, extensionCount, glfwExtensions };

#ifndef DIST
        TK_ASSERT(VulkanDevice::checkForValidationLayerSupport());
        instanceCreateInfo.setPEnabledLayerNames(validationLayers);
#endif

        VulkanRenderer::instance = vk::createInstance(instanceCreateInfo);
    }

    void VulkanRenderer::createSurface() {
        TK_ASSERT(glfwCreateWindowSurface(static_cast<VkInstance>(instance), Application::getNativeWindow(), nullptr, reinterpret_cast<VkSurfaceKHR*>(&VulkanRenderer::surface)) == VK_SUCCESS);
    }

    void VulkanRenderer::createCommandPool() {
        vk::CommandPoolCreateInfo commandPoolCreateInfo{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer, VulkanDevice::getQueueFamilyIndexes().graphicsQueueIndex};
        TK_ASSERT(device.createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool, {}) == vk::Result::eSuccess);
    }

    void VulkanRenderer::createSwapchain() {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
        const auto [graphicsQueueIndex, presentQueueIndex] = VulkanDevice::getQueueFamilyIndexes();
        const auto [windowWidth, windowHeight] = Application::getWindow()->getWindowDimensions();

        std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(surface);
        swapchainImageFormat = VulkanDevice::findSurfaceFormat(formats).format;

        vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
        swapchainExtent = VulkanDevice::getExtent(surfaceCapabilities);

        // TODO: use this for FPS limits later
        vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;
        vk::SurfaceTransformFlagBitsKHR preTransform = VulkanDevice::getPreTransform(surfaceCapabilities);
        vk::CompositeAlphaFlagBitsKHR compositeAlpha = VulkanDevice::getCompositeAlpha(surfaceCapabilities);
        uint32_t imageCount = VulkanDevice::getImageCount(surfaceCapabilities);

        vk::SwapchainCreateInfoKHR swapchainCreateInfo;
        swapchainCreateInfo.flags = {};
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = swapchainImageFormat;
        swapchainCreateInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        swapchainCreateInfo.imageExtent = swapchainExtent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
        swapchainCreateInfo.compositeAlpha = compositeAlpha;
        swapchainCreateInfo.presentMode = swapchainPresentMode;
        swapchainCreateInfo.clipped = true;
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
        swapchainCreateInfo.preTransform = preTransform;
        swapchainCreateInfo.queueFamilyIndexCount = 0;

        uint32_t queueFamilyIndecies[2] = { graphicsQueueIndex, presentQueueIndex };

        if (queueFamilyIndecies[0] != queueFamilyIndecies[1]) {
            swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            swapchainCreateInfo.queueFamilyIndexCount = 2;
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndecies;
        }

        TK_ASSERT(device.createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapchain) == vk::Result::eSuccess);

        swapchainImages = device.getSwapchainImagesKHR(swapchain);
        swapchainImageViews.resize(swapchainImages.size());

        vk::ImageViewCreateInfo imageViewCreateInfo { {}, {}, vk::ImageViewType::e2D, swapchainImageFormat,
            { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA, },
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        };

        for (uint32_t i = 0; i < surfaceCapabilities.minImageCount; ++i) {
            imageViewCreateInfo.image = swapchainImages[i];
            TK_ASSERT(device.createImageView(&imageViewCreateInfo, nullptr, &swapchainImageViews[i]) == vk::Result::eSuccess);
        }
    }

    void VulkanRenderer::createRenderPass() {
        const auto [graphicsQueueIndex, presentQueueIndex] = VulkanDevice::getQueueFamilyIndexes();

        vk::AttachmentDescription colorAttachment(
            vk::AttachmentDescriptionFlags(),
            swapchainImageFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR
        );

        vk::AttachmentDescription depthAttachment(
            vk::AttachmentDescriptionFlags(),
            depthFormat,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthStencilAttachmentOptimal
        );

        vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
        vk::AttachmentReference depthReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, colorReference, {}, &depthReference);

        vk::PipelineStageFlagBits colorDepStageBitMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::SubpassDependency colorDependency(VK_SUBPASS_EXTERNAL, 0, colorDepStageBitMask, colorDepStageBitMask, {}, vk::AccessFlagBits::eColorAttachmentWrite);
        vk::PipelineStageFlagBits depthDepStageBitMask = static_cast<vk::PipelineStageFlagBits>(static_cast<uint32_t>(vk::PipelineStageFlagBits::eEarlyFragmentTests) | static_cast<uint32_t>(vk::PipelineStageFlagBits::eLateFragmentTests));
        vk::SubpassDependency depthDependency(VK_SUBPASS_EXTERNAL, 0, depthDepStageBitMask, depthDepStageBitMask, {}, vk::AccessFlagBits::eDepthStencilAttachmentWrite);

        std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        std::array<vk::SubpassDependency, 2> dependencies = { colorDependency, depthDependency };
        std::array<vk::SubpassDescription, 1> subpasses = { subpass };

        vk::RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo.setAttachments(attachments);
        renderPassCreateInfo.setDependencies(dependencies);
        renderPassCreateInfo.setSubpasses(subpasses);

        TK_ASSERT(device.createRenderPass(&renderPassCreateInfo, nullptr, &renderPass) == vk::Result::eSuccess);
    }

    void VulkanRenderer::createFrames() {
        const auto [graphicsQueueIndex, presentQueueIndex] = VulkanDevice::getQueueFamilyIndexes();

        vk::FenceCreateInfo fenceCreateInfo { vk::FenceCreateFlagBits::eSignaled };
        vk::SemaphoreCreateInfo semaphoreCreateInfo;

        for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
            vk::CommandBufferAllocateInfo commandBufferAllocateInfo{ commandPool, vk::CommandBufferLevel::ePrimary, 1 };
            TK_ASSERT(device.allocateCommandBuffers(&commandBufferAllocateInfo, &frames[i].commandBuffer) == vk::Result::eSuccess);
            TK_ASSERT(device.createFence(&fenceCreateInfo, nullptr, &frames[i].renderFence) == vk::Result::eSuccess);
            TK_ASSERT(device.createSemaphore(&semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore) == vk::Result::eSuccess);
            TK_ASSERT(device.createSemaphore(&semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore) == vk::Result::eSuccess);
        }
    }

    void VulkanRenderer::createDepthBuffer() {
        const auto [graphicsQueueIndex, presentQueueIndex] = VulkanDevice::getQueueFamilyIndexes();
        const auto [windowWidth, windowHeight] = Application::getWindow()->getWindowDimensions();

        vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
        depthFormat = VulkanDevice::findSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
            vk::ImageTiling::eOptimal,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );

        vk::Extent2D extent = VulkanDevice::getExtent(surfaceCapabilities);
        vk::ImageTiling tiling = VulkanDevice::findTiling(depthFormat);

        vk::ImageCreateInfo imageCreateInfo({}, vk::ImageType::e2D, depthFormat, vk::Extent3D{ extent, 1 }, 1, 1, vk::SampleCountFlagBits::e1, tiling, vk::ImageUsageFlagBits::eDepthStencilAttachment);

        depthBuffer.image = device.createImage(imageCreateInfo);

        vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
        vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(depthBuffer.image);
        vk::MemoryAllocateInfo memoryAllocateInfo { memoryRequirements.size, VulkanDevice::findMemoryType(memoryRequirements.memoryTypeBits) };

        TK_ASSERT(device.allocateMemory(&memoryAllocateInfo, nullptr, &depthBuffer.memory) == vk::Result::eSuccess);
        device.bindImageMemory(depthBuffer.image, depthBuffer.memory, 0);

        vk::ImageViewCreateInfo imageViewCreateInfo { {}, depthBuffer.image, vk::ImageViewType::e2D, depthFormat, {}, { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 } };
        TK_ASSERT(device.createImageView(&imageViewCreateInfo, nullptr, &depthBuffer.imageView) == vk::Result::eSuccess);
    }

}