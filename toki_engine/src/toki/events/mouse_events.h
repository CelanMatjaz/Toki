#pragma once

#include "events/event.h"
#include "input/input.h"

namespace toki {

class MouseButtonEvent : public Event {
public:
    MouseButtonEvent(EventType type): Event(type) {}

    u8 get_button() {
        return m_data.u8[0];
    }

    glm::ivec2 get_position() {
        return { m_data.u16[2], m_data.i16[3] };
    }

    KeyboardMods get_mods() {
        return (KeyboardMods) m_data.u16[1];
    }
};

class MousePressEvent : public MouseButtonEvent {
public:
    MousePressEvent(): MouseButtonEvent(EventType::MousePress) {}
};

class MouseReleaseEvent : public MouseButtonEvent {
public:
    MouseReleaseEvent(): MouseButtonEvent(EventType::MouseRelease) {}
};

class MouseMoveEvent : public Event {
public:
    MouseMoveEvent(): Event(EventType::MouseMove) {}

    glm::ivec2 get_position() {
        return { m_data.i32[0], m_data.i32[1] };
    }
};

class MouseScrollEvent : public MouseButtonEvent {
public:
    MouseScrollEvent(): MouseButtonEvent(EventType::MouseMove) {}

    glm::fvec2 get_dimensions() {
        return { m_data.f32[0], m_data.f32[1] };
    }
};

}  // namespace toki
