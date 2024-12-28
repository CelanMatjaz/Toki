#pragma once

#include <memory>

#include "core/macros.h"
#include "engine/window.h"
#include "renderer_state.h"

namespace toki {

class Renderer {
public:
    struct Config {
        std::shared_ptr<Window> initialWindow;
    };

public:
    Renderer() = delete;
    Renderer(const Config& config);
    ~Renderer();

    DELETE_COPY(Renderer);
    DELETE_MOVE(Renderer);

public:  // API
    void add_window(std::shared_ptr<Window> window);

private:
    RendererContext m_context;
};

}  // namespace toki
