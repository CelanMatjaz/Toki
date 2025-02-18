#pragma once

#include <type_traits>

#include "core/base.h"
#include "input/input.h"

namespace toki {

enum class EventType {
    KeyPress,
    KeyRelease,
    KeyRepeat,

    MousePress,
    MouseRelease,
    MouseMove,
    MouseScroll,

    WindowMove,
    WindowResize,
    WindowMaximize,
    WindowMinimize,
    WindowRestore,
    WindowFocus,
    WindowBlur,
    WindowClose,
    WindowEnter,
    WindowLeave,

    EVENT_COUNT
};

union EventData {
    uint64_t u64;
    int64_t i64;

    uint32_t u32[2];
    int32_t i32[2];

    uint16_t u16[4];
    int16_t i16[4];

    uint8_t u8[8];
    int8_t i8[8];

    float f32[2];
};

class Event;
using EventFunction = void (*)(void* sender, void* receiver, Event&);

class Event {
public:
    Event() = delete;
    Event(EventType event_type, EventData data = {});
    ~Event() = default;

    EventType get_type() const;
    EventData get_data() const;
    b8 is_handled() const;
    void set_handled();

    template <typename T>
        requires std::derived_from<T, Event>
    T& as() const {
        return (T&) *this;
    }

    bool operator==(const EventType& type) const {
        return m_eventType == type;
    }

    operator EventType() {
        return m_eventType;
    }

protected:
    EventType m_eventType;
    EventData m_data;
    b8 m_isHandled;
};

Event create_key_event(EventType type, int key, int scancode, KeyboardMods mods);
Event create_mouse_button_event(EventType type, int button, KeyboardMods mods, double xpos, double ypos);
Event create_mouse_move_event(double xpos, double ypos);
Event create_mouse_scroll_event(double xoffset, double yoffset);
Event create_window_move_event(double xpos, double ypos);
Event create_window_resize_event(int width, int height);

}  // namespace toki
