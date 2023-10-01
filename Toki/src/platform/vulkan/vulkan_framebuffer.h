#pragma once

#include "renderer/framebuffer.h"
#include "platform/vulkan/vulkan_render_pass.h"
#include "platform/vulkan/backend/vulkan_context.h"
#include "platform/vulkan/backend/vulkan_image.h"
#include "vulkan/vulkan.h"
#include "vector"

namespace Toki {

    class VulkanFramebuffer : public Framebuffer {
    public:
        VulkanFramebuffer(const FramebufferConfig& config);
        ~VulkanFramebuffer();

        virtual void bind() override;
        virtual void unbind() override;
        virtual void resize(uint32_t width, uint32_t height, uint32_t layers = 1) override;
        virtual void nextSubpass() override;
        virtual float readPixel(uint32_t attachmentIndex, uint32_t x, uint32_t y, uint32_t z) override;
        virtual Ref<Texture> getAttachment(uint32_t attachmentIndex) override;

        bool isSwapchainTarget() { return config.target == RenderTarget::Swapchain; }

    private:
        void create();
        void destroy();

        std::vector<Ref<VulkanImage>> attachments[VulkanContext::MAX_FRAMES];
        VkFramebuffer framebuffers[VulkanContext::MAX_FRAMES];
    };

}