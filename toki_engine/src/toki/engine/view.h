#pragma once

#include "core/macros.h"
#include "engine/system_manager.h"
#include "events/event.h"
#include "renderer/renderer.h"

namespace toki {

class View {
    friend class Engine;

public:
    View() = default;
    virtual ~View() = default;

    DELETE_COPY(View)
    DELETE_MOVE(View)

    virtual void on_add() {};
    virtual void on_destroy() {};
    virtual void on_render() {};
    virtual void on_update([[maybe_unused]] float delta_time) {};
    virtual void on_event([[maybe_unused]] Event& event) {};

protected:
    Systems& get_systems() const;
    const Input& get_input() const;
    Renderer& get_renderer() const;

private:
    Engine* m_engine;
};

}  // namespace toki
