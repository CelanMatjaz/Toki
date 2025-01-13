#pragma once

#include "engine/window.h"
#include "renderer/configs.h"
#include "renderer/renderer_api.h"

namespace toki {

class Engine;
struct RendererContext;

class Renderer {
    friend Engine;

public:
    struct Config {
        std::shared_ptr<Window> initialWindow;
    };

protected:
    static std::shared_ptr<Renderer> create(const Config& config);

    Renderer() = delete;
    Renderer(const Config& config);
    ~Renderer() = default;

    DELETE_COPY(Renderer);
    DELETE_MOVE(Renderer);

    std::shared_ptr<RendererApi> get_renderer_api() const;

public:
    virtual Handle create_shader(const ShaderCreateConfig& config) = 0;
    virtual void destroy_shader(Handle shader_handle) = 0;

    virtual Handle create_buffer(const BufferCreateConfig& config) = 0;
    virtual void destroy_buffer(Handle buffer_handle) = 0;

    virtual Handle create_texture(const TextureCreateConfig& config) = 0;
    virtual Handle create_texture_from_file(std::string_view path) = 0;
    virtual void destroy_texture(Handle texture_handle) = 0;

    virtual Handle create_framebuffer(const FramebufferCreateConfig& config) = 0;
    virtual void destroy_framebuffer(Handle framebuffer_handle) = 0;

    virtual void set_buffer_data(Handle buffer_handle, u32 size, void* data) = 0;
    virtual void set_texture_data(Handle texture_handle, u32 size, void* data) = 0;

protected:
    virtual b8 begin_frame() = 0;
    virtual void end_frame() = 0;
    virtual void present() = 0;

    virtual void wait_for_resources() = 0;

    void add_window(std::shared_ptr<Window> window);
    std::shared_ptr<RendererContext> m_context;
    std::shared_ptr<RendererApi> m_rendererApi;
};

}  // namespace toki
