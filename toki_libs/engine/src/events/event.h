#pragma once

#include <toki/core.h>

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
    u64 u64;

    struct {
        u8 key_code;
        u8 scan_code;
        u8 mods;
    } key_event_data;

    struct {
        i16 x;
        i16 y;
        u8 button_code;
    } mouse_button_event_data;

    struct {
        i16 x;
        i16 y;
    } mouse_move_event_data;

    struct {
        i16 x;
        i16 y;
    } mouse_scroll_event_data;

    struct {
        i32 x;
        i32 y;
    } window_move_event_data;
};

struct Event {
    static Event create(EventType event_type, EventData data) {
        return Event{ .event_type = event_type, .data = data };
    }

    bool operator==(const EventType& type) const {
        return event_type == type;
    }

    operator EventType() {
        return event_type;
    }

    EventType event_type;
    EventData data;
    b32 is_handled{ false };
};

}  // namespace toki
