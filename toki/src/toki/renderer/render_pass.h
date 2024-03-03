#pragma once

#include <vector>

#include "toki/core/core.h"
#include "toki/renderer/renderer_types.h"

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

private:
    RenderPass() = default;
};

}  // namespace Toki
