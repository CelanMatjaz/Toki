#pragma once

#include "engine/window.h"
#include "renderer/shader.h"

namespace toki {

struct RendererContext;

class Renderer {
public:
    struct Config {
        std::shared_ptr<Window> initialWindow;
    };

public:
    static std::shared_ptr<Renderer> create(const Config& config);

    Renderer() = delete;
    Renderer(const Config& config);
    ~Renderer() = default;

    DELETE_COPY(Renderer);
    DELETE_MOVE(Renderer);

public:  // API
    virtual std::shared_ptr<Shader> create_shader(const Shader::Config& config) const = 0;

protected:
    void add_window(std::shared_ptr<Window> window);

protected:
    std::unique_ptr<RendererContext> m_context{};
};

}  // namespace toki
