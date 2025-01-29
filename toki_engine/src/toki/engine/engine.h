#pragma once

#include "core/macros.h"
#include "engine/system_manager.h"
#include "engine/view.h"
#include "engine/window.h"
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
    void add_view(View* view);

    Renderer& get_renderer() const {
        return *m_renderer;
    }

    SystemManager& get_system_manager() const {
        return *m_systemManager;
    }

    const Input& get_input() const {
        return m_windows.front()->get_input();
    }

private:
    void handle_event(Event& event);

    b8 m_isRunning = false;

    std::vector<Window*> m_windows;
    std::vector<View*> m_views;
    Renderer* m_renderer;

    StackAllocator m_engineAllocator;

    // Systems
    SystemManager* m_systemManager{};
    StackAllocator m_systemAllocator;
};

}  // namespace toki
