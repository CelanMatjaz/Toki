#pragma once

#include <toki/core.h>

#include <memory>

#include "../renderer/renderer.h"
#include "window.h"

namespace toki {

class Engine {
public:
    struct Config {};

public:
    Engine() = delete;
    Engine(const Config& config);
    ~Engine();

    DELETE_COPY(Engine);
    DELETE_MOVE(Engine);

    void run();

private:
    bool m_isRunning = false;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
};

}  // namespace toki
