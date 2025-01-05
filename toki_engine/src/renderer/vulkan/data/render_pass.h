#pragma once

#include <vulkan/vulkan.h>

#include "core/scope_wrapper.h"
#include "renderer/renderer_types.h"
#include "renderer/vulkan/data/attachment_hash.h"
#include "renderer/vulkan/renderer_state.h"

namespace toki {

class RenderPass {
public:
    struct Config {
        std::vector<Attachment> attachments;
        VkFormat presentFormat{};
    };

    static Ref<RenderPass> create(Ref<RendererContext> ctx, const Config& config);

    RenderPass() = delete;
    RenderPass(Ref<RendererContext> ctx, const Config& config);
    ~RenderPass();

    inline static Ref<RendererContext> s_context;

    const AttachmentsHash get_attachment_hash() const;
    operator VkRenderPass() const;

private:
    VkRenderPass m_renderPass{};
    AttachmentsHash m_attachmentHash{};
};

}  // namespace toki
