#pragma once

#include "core/macros.h"
#include "engine/window.h"
#include "renderer/renderer_commands.h"
#include "renderer/renderer_types.h"

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

    Framebuffer create_framebuffer(const FramebufferConfig& config);
    void destroy_framebuffer(Framebuffer* framebuffer);

    Buffer create_buffer(const BufferConfig& config);
    void destroy_buffer(Buffer* buffer);
    void set_bufffer_data(Buffer* buffer, u32 size, void* data);

    Texture create_texture(const TextureConfig& config);
    void destroy_texture(Texture* texture);

    Shader create_shader(const Framebuffer* framebuffer, const ShaderConfig& config);
    void destroy_shader(Shader* shader);

    void wait_for_resources();

public:
    virtual ~Renderer() = default;
    DELETE_COPY(Renderer);
    DELETE_MOVE(Renderer);

protected:
    void* m_backend;
};

}  // namespace toki
