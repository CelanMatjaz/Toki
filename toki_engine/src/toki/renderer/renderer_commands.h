#pragma once

#include <functional>

#include "core/math_types.h"
#include "renderer/renderer_structs.h"

namespace toki {

class RendererCommands {
public:
    void begin_pass();
    void end_pass();

    void set_viewport(const Rect2D& rect);
    void reset_viewport();
    void set_scissor(const Rect2D& rect);
    void reset_scissor();

    void bind_shader(Shader const& shader);
    void bind_buffer(Buffer const& buffer);

    void draw(u32 count);
    void draw_indexed(u32 count);
    void draw_instanced(u32 index_count, u32 instance_count = 1);

private:
    void* m_backend;
};

using SubmitFn = std::function<void(RendererCommands&)>;

}  // namespace toki
