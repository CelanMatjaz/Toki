#pragma once

#include <memory>

#include "core/macros.h"
#include "engine/window.h"
#include "renderer/vulkan/swapchain.h"
#include "renderer_state.h"

namespace toki {

class Engine;

class Renderer {
    friend Engine;

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
private:
    void add_window(std::shared_ptr<Window> window);

    RendererContext m_context;
    std::vector<std::shared_ptr<Swapchain>> m_swapchains;
};

}  // namespace toki
