#pragma once

#include "core/base.h"

namespace toki {

enum class EventType {
    KeyPress,
    KeyRelease,
    KeyRepeat,

    MouseClick,
    MouseRelease,
    MouseHold,
    MouseMove,
    MouseScroll,
    MouseEnter,
    MouseLeave,

    WindowMove,
    WindowResize,
    WindowMaximize,
    WindowMinimize,
    WindowRestore,
    WindowFocus,
    WindowBlur,
    WindowClose,

    EVENT_COUNT
};

union EventData {
    u64 u64;
    i64 i64;

    u32 u32[2];
    i32 i32[2];

    u16 u16[4];
    i16 i16[4];

    u8 u8[8];
    i8 i8[8];

    f32 f32[2];
};

class Event;
using EventFunction = std::function<void(void* sender, void* receiver, const Event&)>;

class Event {
public:
    Event() = delete;
    Event(EventType event_type, EventData data = {});
    ~Event() = default;

    EventType get_type() const;
    EventData get_data() const;
    b8 is_handled() const;
    void set_handled();

private:
    EventType m_eventType;
    EventData m_data;
    b8 m_isHandled;
};

Event create_key_event(EventType type, int key, int scancode, int action, int mods);
Event create_mouse_button_event(EventType type, int button, int action, int mods, double xpos, double ypos);
Event create_mouse_move_event(double xpos, double ypos);
Event create_mouse_scroll_event(double xoffset, double yoffset);
Event create_window_move_event(double xpos, double ypos);
Event create_window_resize_event(int width, int height);

}  // namespace toki
