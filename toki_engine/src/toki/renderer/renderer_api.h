#pragma once

#include "core/base.h"
#include "renderer/renderer_types.h"

namespace toki {

class renderer_api {
public:
    virtual void begin_pass(const begin_pass_config& config) = 0;
    virtual void end_pass() = 0;
    virtual void submit() = 0;

    virtual void bind_shader(handle handle) = 0;

    virtual void reset_viewport() = 0;
    virtual void reset_scissor() = 0;
};

}  // namespace toki
