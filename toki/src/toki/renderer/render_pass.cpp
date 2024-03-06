#include "render_pass.h"

#include "renderer/vulkan_render_pass.h"

namespace Toki {

Ref<RenderPass> RenderPass::create(const RenderPassConfig& config) {
    return createRef<VulkanRenderPass>(config);
}

}  // namespace Toki