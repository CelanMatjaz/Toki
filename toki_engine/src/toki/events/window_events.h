#pragma once

#include "events/event.h"

namespace toki {

class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(): Event(EventType::WindowResize) {}

    glm::uvec2 get_dimensions() const {
        return { m_data.u32[0], m_data.u32[1] };
    }
};

}  // namespace toki
