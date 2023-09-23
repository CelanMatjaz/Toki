#include "tkpch.h"
#include "framebuffer.h"
#include "renderer.h"

#include "platform/vulkan/vulkan_framebuffer.h"

namespace Toki {

    Ref<Framebuffer> Framebuffer::create(const FramebufferConfig& config) {
        return createRef<VulkanFramebuffer>(config);
    }

    Framebuffer::Framebuffer(const FramebufferConfig& config) : config(config) {}

}