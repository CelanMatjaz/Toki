#pragma once

#include "core/base.h"
#include "renderer/renderer_types.h"

namespace toki {

class RendererApi {
public:
    virtual void begin_pass(const BeginPassConfig& config) = 0;
    virtual void end_pass() = 0;
    virtual void submit() = 0;

    virtual void bind_shader(Handle handle) = 0;

    virtual void reset_viewport() = 0;
    virtual void reset_scissor() = 0;
};

}  // namespace toki
