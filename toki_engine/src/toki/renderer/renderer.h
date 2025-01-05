#pragma once

#include "core/id.h"
#include "engine/window.h"
#include "renderer/renderer_api.h"
#include "renderer/shader.h"

namespace toki {

struct Engine;
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
    virtual Handle create_shader(const Shader::Config& config) = 0;

protected:
    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;
    virtual void present() = 0;

    void add_window(std::shared_ptr<Window> window);
    std::shared_ptr<RendererContext> m_context{};
    std::shared_ptr<RendererApi> m_rendererApi{};
};

}  // namespace toki
