#include "tkpch.h"
#include "vulkan_frame_buffer.h"

#include "toki/core/application.h"
#include "infos.h"

namespace Toki {

    void VulkanFrameBuffer::cleanup() {
        VkDevice device = Application::getVulkanRenderer()->getDevice();

        vkDestroyFramebuffer(device, frameBuffer, nullptr);
    }

    VulkanFrameBuffer VulkanFrameBuffer::create(VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& attachments) {
        VkDevice device = Application::getVulkanRenderer()->getDevice();

        VkFramebufferCreateInfo framebufferCreateInfo = Infos::Renderer::framebufferCreateInfo(renderPass, extent.width, extent.height, 1, attachments);

        VulkanFrameBuffer frameBuffer;
        TK_ASSERT_VK_RESULT(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &frameBuffer.frameBuffer));

        return frameBuffer;
    }

}