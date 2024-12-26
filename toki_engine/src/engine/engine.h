#pragma once

#include <toki/core.h>
#include <toki/renderer.h>

#include <memory>

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
    bool m_is_running = false;

    std::shared_ptr<Window> m_window;
    std::shared_ptr<Renderer> m_renderer;
};

}  // namespace toki
