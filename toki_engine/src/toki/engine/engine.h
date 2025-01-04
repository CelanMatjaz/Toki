#pragma once

#include <memory>

#include "engine/window.h"
#include "renderer/renderer.h"

namespace toki {

class Engine {
public:
    struct Config {
        std::shared_ptr<Window> initialWindow;
    };

public:
    Engine() = delete;
    Engine(const Config& config);
    ~Engine();

    DELETE_COPY(Engine);
    DELETE_MOVE(Engine);

    void run();

private:
    bool m_isRunning = false;

    std::vector<std::shared_ptr<Window>> m_windows;
    std::shared_ptr<Renderer> m_renderer;
};

}  // namespace toki
