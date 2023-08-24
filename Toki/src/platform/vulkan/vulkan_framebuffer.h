#pragma once

#include "renderer/framebuffer.h"
#include "platform/vulkan/backend/vulkan_context.h"
#include "platform/vulkan/backend/vulkan_image.h"
#include "platform/vulkan/backend/vulkan_render_pass.h"
#include "vulkan/vulkan.h"
#include "vector"

namespace Toki {

    struct VulkanFramebufferConfig {
        std::vector<VkImageView> attachments;
        uint32_t width = 1280, height = 720;
    };

    class VulkanFramebuffer : public Framebuffer {
    public:
        VulkanFramebuffer(const FramebufferConfig& config);
        ~VulkanFramebuffer();

        virtual void bind() override;
        virtual void unbind() override;

        VkRenderPass getRenderPass() { return renderPass->getHandle(); }

    private:
        void create();
        void destroy();

        Ref<VulkanRenderPass> renderPass;
        std::vector<Ref<VulkanImage>> attachments[VulkanContext::MAX_FRAMES];
        VkFramebuffer framebuffers[VulkanContext::MAX_FRAMES];
    };

}