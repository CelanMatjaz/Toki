#pragma once

#include <vulkan/vulkan.h>

#include "core/scope_wrapper.h"
#include "renderer/renderer_types.h"
#include "renderer/vulkan/data/attachment_hash.h"

namespace toki {

struct RendererContext;

class RenderPass {
public:
    struct Config {
        std::vector<Attachment> attachments;
        VkFormat presentFormat{};
    };

    static RenderPass create(Ref<RendererContext> ctx, const Config& config);
    static void cleanup(Ref<RendererContext> ctx, RenderPass& render_pass);

    VkRenderPass renderPass{};
    AttachmentsHash attachmentHash{};
};

}  // namespace toki
