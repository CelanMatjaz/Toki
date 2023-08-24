#include "tkpch.h"
#include "vulkan_renderer.h"
#include "core/assert.h"
#include "vulkan/vulkan.h"

namespace Toki {

    Ref<VulkanContext> VulkanRenderer::context;

    void VulkanRenderer::init() {
        VulkanRenderer::context = createRef<VulkanContext>();
        VulkanRenderer::context->init();
    }

    void VulkanRenderer::shutdown() {
        vkDeviceWaitIdle(context->device);
        VulkanRenderer::context->shutdown();
        VulkanRenderer::context.reset();
    }

    void VulkanRenderer::beginFrame() {
        VkDevice device = context->device;

        VulkanContext::FrameData* frame = context->getCurrentFrame();
        uint64_t timeout = UINT64_MAX;

        VkResult result = vkAcquireNextImageKHR(device, context->swapchain->getHandle(), timeout, frame->presentSemaphore, nullptr, &context->imageIndex);

        VkResult waitFencesResult = vkWaitForFences(device, 1, &frame->renderFence, VK_TRUE, 1000000000);
        TK_ASSERT(waitFencesResult == VK_SUCCESS || waitFencesResult == VK_TIMEOUT, "Failed waiting for fences");

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            context->swapchain->recreate();
            beginFrame();
            return;
        }
        else TK_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire swapchain image");

        TK_ASSERT_VK_RESULT(vkResetFences(device, 1, &frame->renderFence), "Error resetting fences");
        TK_ASSERT((result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR), "Failed to acquire swapchain image");

        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        TK_ASSERT_VK_RESULT(vkBeginCommandBuffer(frame->commandBuffer, &commandBufferBeginInfo), "Error starting command buffer recording");
    }

    void VulkanRenderer::endFrame() {
        VkDevice device = context->device;
        VulkanContext::FrameData* frame = context->getCurrentFrame();

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

        TK_ASSERT_VK_RESULT(vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, frame->renderFence), "Failed to submit queue");
        TK_ASSERT_VK_RESULT(vkWaitForFences(device, 1, &frame->renderFence, true, UINT64_MAX), "Failed to reset fences");

        VkSwapchainKHR swapchains[] = { context->swapchain->getHandle() };
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains = swapchains;
        presentInfo.swapchainCount = 1;
        presentInfo.pImageIndices = &context->imageIndex;
        presentInfo.pWaitSemaphores = &frame->renderSemaphore;
        presentInfo.waitSemaphoreCount = 1;

        VkResult result = vkQueuePresentKHR(context->presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            context->swapchain->recreate();
            return;
        }
        else TK_ASSERT_VK_RESULT(result, "Failed to present swapchain image");

        context->currentFrame = ++context->currentFrame % VulkanContext::MAX_FRAMES;
    }

}