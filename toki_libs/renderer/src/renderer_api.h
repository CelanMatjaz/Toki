#pragma once

#include <toki/core.h>

#include "renderer_commands.h"
#include "renderer_types.h"

namespace toki {

template <typename F>
concept SubmitFunctionConcept = requires(F fn, RendererCommands& commands) {
    { fn(commands) } -> SameAsConcept<void>;
};

class RendererApi {
public:
    struct Config {};

    RendererApi() = delete;
    RendererApi(const Config& config);
    virtual ~RendererApi() = 0;

public:
    void begin_frame();
    void end_frame();
    void present();

    void submit(SubmitFunctionConcept auto submit);

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
    DELETE_COPY(RendererApi);
    DELETE_MOVE(RendererApi);

protected:
    void* m_backend;
};

}  // namespace toki
