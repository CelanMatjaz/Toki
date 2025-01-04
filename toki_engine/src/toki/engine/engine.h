#pragma once

#include <memory>

#include "engine/view.h"
#include "engine/window.h"
#include "renderer/renderer.h"

namespace toki {

class Engine {
public:
    struct Config {
        Ref<Window> initialWindow;
    };

public:
    Engine() = delete;
    Engine(const Config& config);
    ~Engine();

    DELETE_COPY(Engine);
    DELETE_MOVE(Engine);

    void run();

    void add_view(Ref<View> view);

private:
    bool m_isRunning = false;

    std::vector<Ref<Window>> m_windows;
    std::vector<Ref<View>> m_views;
    Ref<Renderer> m_renderer;
};

}  // namespace toki
