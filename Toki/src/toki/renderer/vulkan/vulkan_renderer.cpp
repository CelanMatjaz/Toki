#include "vulkan_renderer.h"

#include "tkpch.h"
#include "toki/core/application.h"
#include "vulkan_constants.h"
#include "vulkan_pipeline.h"
#include "infos.h"

namespace Toki {

    VulkanRenderer::VulkanRenderer() { }

    VulkanRenderer::~VulkanRenderer() {
        shutdown();
    }

    void VulkanRenderer::init() {
        device = new VulkanDevice();
        swapchain = new VulkanSwapchain();

        const auto [graphicsQueueIndex, presentQueueIndex] = device->getQueueFamilyIndexes();
        vkGetDeviceQueue(getDevice(), graphicsQueueIndex, 0, &graphicsQueue);
        vkGetDeviceQueue(getDevice(), presentQueueIndex, 0, &presentQueue);
        createCommandPool();
        createFrames();
    }

    void VulkanRenderer::shutdown() {
        VkDevice device = this->getDevice();
        vkDeviceWaitIdle(device);

        for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
            frames[i].cleanup();
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        delete this->swapchain;
        delete this->device;
    }

    void VulkanRenderer::beginFrame() {
        VkDevice device = this->getDevice();

        FrameData* frame = getCurrentFrame();
        uint64_t timeout = UINT64_MAX - 1;
        VkResult result = vkAcquireNextImageKHR(device, swapchain->getSwapchainHandle(), timeout, frame->presentSemaphore, nullptr, &imageIndex);

        TK_ASSERT(vkWaitForFences(device, 1, &frame->renderFence, VK_TRUE, timeout) == VK_SUCCESS);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            swapchain->recreate();
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
        clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
        VkClearValue depthClear;
        depthClear.depthStencil.depth = 1.f;

        std::array<VkClearValue, 2> clearValues = { clearValue, depthClear };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = swapchain->getRenderPass();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = swapchain->getExtent();
        renderPassBeginInfo.framebuffer = swapchain->getFrameBuffers()[imageIndex].getFrameBuffer();
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        swapchain->getRenderPassHandle().beginRenderPass(frame->commandBuffer, &renderPassBeginInfo);
    }

    void VulkanRenderer::endFrame() {
        TK_ASSERT(isFrameStarted);
        VkDevice device = this->device->getDevice();

        FrameData* frame = getCurrentFrame();
        swapchain->getRenderPassHandle().endRenderPass(frame->commandBuffer);

        TK_ASSERT(isFrameStarted && "Can't call endFrame while frame is not in progress");
        vkEndCommandBuffer(frame->commandBuffer);

        VkSemaphore waitSemaphores[] = { frame->presentSemaphore };
        VkSemaphore signalSemaphores[] = { frame->renderSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame->commandBuffer;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        TK_ASSERT(vkResetFences(device, 1, &frame->renderFence) == VK_SUCCESS);
        TK_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, frame->renderFence) == VK_SUCCESS);

        VkSwapchainKHR swapchains[] = { swapchain->getSwapchainHandle() };
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains = swapchains;
        presentInfo.swapchainCount = 1;
        presentInfo.pImageIndices = &this->imageIndex;
        presentInfo.pWaitSemaphores = &frame->renderSemaphore;
        presentInfo.waitSemaphoreCount = 1;

        VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Application::getWindow()->wasResized() || shouldRecreateSwapchain) {
            Application::getWindow()->resetResizedFlag();
            swapchain->recreate(newPresentMode);
            shouldRecreateSwapchain = false;
        }
        else if (result != VK_SUCCESS) {
            TK_ASSERT(false && "Failed to present swapchain image");
        }

        isFrameStarted = false;
        currentFrame = ++currentFrame % MAX_FRAMES;
    }

    void VulkanRenderer::onResize() {
        swapchain->recreate();
    }

    void VulkanRenderer::createCommandPool() {
        VkCommandPoolCreateInfo commandPoolCreateInfo = Infos::Renderer::commandPoolCreateInfo(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, device->getQueueFamilyIndexes().graphicsQueueIndex);
        TK_ASSERT(vkCreateCommandPool(device->getDevice(), &commandPoolCreateInfo, nullptr, &commandPool) == VK_SUCCESS);
    }

    void VulkanRenderer::createFrames() {
        const auto [graphicsQueueIndex, presentQueueIndex] = device->getQueueFamilyIndexes();
        VkDevice device = this->device->getDevice();

        VkFenceCreateInfo fenceCreateInfo = Infos::Renderer::fenceCreateInfo();
        VkSemaphoreCreateInfo semaphoreCreateInfo = Infos::Renderer::semaphoreCreateInfo();

        for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
            VkCommandBufferAllocateInfo commandBufferAllocateInfo = Infos::Renderer::commandBufferAllocateInfo(commandPool);
            TK_ASSERT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &frames[i].commandBuffer) == VK_SUCCESS);
            TK_ASSERT(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence) == VK_SUCCESS);
            TK_ASSERT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore) == VK_SUCCESS);
            TK_ASSERT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore) == VK_SUCCESS);
        }
    }

    VkCommandBuffer VulkanRenderer::startCommandBuffer() {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = Infos::Renderer::commandBufferAllocateInfo(commandPool);

        VkCommandBuffer commandBuffer;
        TK_ASSERT(vkAllocateCommandBuffers(device->getDevice(), &commandBufferAllocateInfo, &commandBuffer) == VK_SUCCESS);

        VkCommandBufferBeginInfo commandBufferBeginInfo = Infos::Renderer::commandBufferBeginInfo();
        TK_ASSERT(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) == VK_SUCCESS);

        return commandBuffer;
    }

    void VulkanRenderer::endCommandBuffer(VkCommandBuffer commandBuffer) {
        VkDevice device = this->device->getDevice();
        TK_ASSERT(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        TK_ASSERT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
        TK_ASSERT(vkQueueWaitIdle(graphicsQueue) == VK_SUCCESS);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

}