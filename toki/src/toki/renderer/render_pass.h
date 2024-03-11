#pragma once

#include <vector>

#include "toki/core/core.h"
#include "toki/renderer/renderer_types.h"
#include "toki/renderer/rendering_context.h"

namespace Toki {

struct RenderPassConfig {
    uint16_t width, height;
    std::vector<Attachment> attachments;
};

class RenderPass {
public:
    static Ref<RenderPass> create(const RenderPassConfig& config);

    RenderPass(const RenderPass& other) = delete;
    RenderPass(RenderPass&& other) = delete;
    RenderPass& operator=(const RenderPass& other) = delete;
    RenderPass& operator=(const RenderPass&& other) = delete;
    virtual ~RenderPass() = default;

    virtual Ref<Texture> getColorAttachment(uint32_t textureIndex) = 0;
    virtual Ref<Texture> getDepthAttachment() = 0;
    virtual Ref<Texture> getStencilAttachment() = 0;

protected:
    RenderPass() = default;
};

}  // namespace Toki
