#include "tkpch.h"
#include "framebuffer.h"
#include "renderer.h"
#include "platform/vulkan/vulkan_framebuffer.h"
#include "platform/vulkan/vulkan_renderer.h"

namespace Toki {

    Ref<Framebuffer> Framebuffer::create(const FramebufferConfig& config) {
        Ref<VulkanFramebuffer> framebuffer = createRef<VulkanFramebuffer>(config);
        if (framebuffer->isSwapchainTarget())
            VulkanRenderer::swapchain()->framebuffers.emplace_back(framebuffer);
        return framebuffer;
    }

    Framebuffer::Framebuffer(const FramebufferConfig& config) : config(config) {}

}