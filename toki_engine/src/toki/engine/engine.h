#pragma once

#include <memory>

#include "engine/view.h"
#include "engine/window.h"
#include "events/event_handler.h"
#include "renderer/renderer.h"

namespace toki {

class Engine {
public:
    struct Config {
        Window::Config window_config;
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
    void handle_event(Event& event);

    b8 m_isRunning = false;

    std::vector<Ref<Window>> m_windows;
    std::vector<Ref<View>> m_views;
    Ref<Renderer> m_renderer;
    Scope<EventHandler> m_eventHandler;
};

}  // namespace toki
