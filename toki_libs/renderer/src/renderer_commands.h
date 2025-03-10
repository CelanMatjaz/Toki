#pragma once

#include "renderer_types.h"

namespace toki {

class RendererCommands {
public:
    virtual void begin_rendering(const Framebuffer* framebuffer, const Rect2D& render_area) = 0;
    virtual void end_rendering() = 0;

    virtual void set_viewport(const Rect2D& rect) = 0;
    virtual void reset_viewport() = 0;
    virtual void set_scissor(const Rect2D& rect) = 0;
    virtual void reset_scissor() = 0;

    virtual void bind_shader(Shader const& shader) = 0;
    virtual void bind_buffer(Buffer const& buffer) = 0;

    virtual void push_constants(u32 offset, u32 size, const void* data) = 0;

    virtual void draw(u32 count) = 0;
    virtual void draw_indexed(u32 count) = 0;
    virtual void draw_instanced(u32 index_count, u32 instance_count = 1) = 0;
};

}  // namespace toki
