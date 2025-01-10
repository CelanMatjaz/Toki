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
    virtual void bind_vertex_buffer(Handle handle) = 0;
    virtual void bind_index_buffer(Handle handle) = 0;

    virtual void reset_viewport() = 0;
    virtual void reset_scissor() = 0;

    virtual void draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) = 0;
    virtual void draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) = 0;
};

}  // namespace toki
