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

protected:
    RenderPass() = default;
};

}  // namespace Toki
