#include "tkpch.h"
#include "render_pass.h"
#include "platform/vulkan/vulkan_render_pass.h"

namespace Toki {

    Ref<RenderPass> RenderPass::create(const RenderPassConfig& config) {
        return createRef<VulkanRenderPass>(config);
    }

    RenderPass::RenderPass(const RenderPassConfig& config) : config(config) {}

}