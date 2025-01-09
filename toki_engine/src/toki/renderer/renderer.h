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

private:
    static std::shared_ptr<Renderer> create(const Config& config);

public:
    Renderer() = delete;
    Renderer(const Config& config);
    ~Renderer() = default;

    DELETE_COPY(Renderer);
    DELETE_MOVE(Renderer);

    std::shared_ptr<RendererApi> get_renderer_api() const;

public:
    virtual Handle create_shader(const shader_create_config& config) = 0;
    virtual void destroy_shader(Handle shader_handle) = 0;

    virtual Handle create_buffer(const buffer_create_config& config) = 0;
    virtual void destroy_buffer(Handle buffer_handle) = 0;

    virtual Handle create_framebuffer(const framebuffer_create_config& config) = 0;
    virtual void destroy_framebuffer(Handle framebuffer_handle) = 0;

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
