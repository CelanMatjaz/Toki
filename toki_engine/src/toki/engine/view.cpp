#include "view.h"

#include "engine/engine.h"

namespace toki {

Systems& View::get_systems() const {
    return m_engine->get_system_manager().get_systems();
}

const Input& View::get_input() const {
    return m_engine->get_input();
}

Renderer& View::get_renderer() const {
    return m_engine->get_renderer();
}

}  // namespace toki
