#pragma once

#include <toki/core.h>

#include "renderer_commands.h"
#include "renderer_types.h"

namespace toki {

namespace renderer {

template <typename F>
concept SubmitFunctionConcept = requires(F fn, renderer::Commands& commands) {
    { fn(commands) } -> SameAsConcept<void>;
};

class RendererFrontend {
public:
    struct Config {};

    RendererFrontend() = delete;
    RendererFrontend(const Config& config);
    ~RendererFrontend();

    DELETE_COPY(RendererFrontend);
    DELETE_MOVE(RendererFrontend);

public:
    void begin_frame();
    void end_frame();
    void present();

    void submit(SubmitFunctionConcept auto submit);

    Framebuffer framebuffer_create(const FramebufferConfig& config);
    void framebuffer_destroy(Framebuffer& framebuffer);
    void framebuffer_resize(Framebuffer& framebuffer, u32 width, u32 height);

    Buffer buffer_create(const BufferConfig& config);
    void buffer_destroy(Buffer* buffer);
    void buffer_set_data(Buffer* buffer, u32 size, void* data);

    Texture texture_create(const TextureConfig& config);
    void texture_destroy(Texture* texture);

    Shader shader_create(const Framebuffer* framebuffer, const ShaderConfig& config);
    void shader_destroy(Shader* shader);

    void wait_for_resources();

protected:
    void* m_backend;
};

}  // namespace renderer

}  // namespace toki
