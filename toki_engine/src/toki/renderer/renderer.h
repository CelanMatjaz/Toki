#pragma once

#include "engine/window.h"
#include "renderer/renderer_commands.h"
#include "renderer/renderer_structs.h"
#include "resources/configs/shader_config_loader.h"

namespace toki {

class Engine;

class Renderer {
    friend Engine;

public:
    struct Config {
        Window* initial_window;
    };

    Renderer() = delete;
    Renderer(const Config& config);

public:
    void begin_frame();
    void end_frame();
    void present();

    void submit(SubmitFn submit_fn);

    void set_color_clear(const glm::vec4& color);
    void set_depth_clear(f32 depth);

    RenderPass create_render_pass();
    void destroy_render_pass(RenderPass& render_pass);

    Buffer create_buffer(BufferType type, u32 size);
    void destroy_buffer(Buffer& buffer);
    void set_bufffer_data(Buffer* buffer, u32 size, void* data);

    Texture create_texture(ColorFormat format, u32 width, u32 height);
    void destroy_texture(Handle texture_handle);

    Shader create_shader(RenderPass& render_pass, configs::ShaderConfig shader_config);
    void destroy_shader(Shader& shader);

    void wait_for_resources();

public:
    virtual ~Renderer() = default;
    DELETE_COPY(Renderer);
    DELETE_MOVE(Renderer);

protected:
    void* m_backend;
};

}  // namespace toki
