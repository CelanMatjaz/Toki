#pragma once

#include <vulkan/vulkan.h>

#include "core/scope_wrapper.h"
#include "renderer/renderer_types.h"
#include "renderer/vulkan/renderer_state.h"

namespace toki {

class RenderPass {
public:
    struct Config {
        std::vector<Attachment> attachments;
        VkFormat presentFormat{};
    };

    RenderPass() = delete;
    RenderPass(RendererContext* ctx, const Config& config);
    ~RenderPass();

private:
    Scoped<VkRenderPass, VK_NULL_HANDLE> m_renderPass2;
    VkRenderPass m_renderPass{};
};

}  // namespace toki
