#pragma once

#include <toki/core.h>

namespace toki {

enum class EventType {
    INVALID,

    // Key events
    KEY_DOWN,
    KEY_UP,
    KEY_RELEASE,

    // Mouse events
    MOUSE_DOWN,
    MOUSE_UP,
    MOUSE_SCROLL,

    // Window events
    WINDOW_RESIZE,
    WINDOW_MAXIMIZE,
    WINDOW_MINIMIZE,
    WINDOW_MOVE,
    WINDOW_FOCUS,
    WINDOW_BLUR,
    WINDOW_CLOSE,

    EVENT_TYPE_COUNT
};

enum KeyMods : u32 {
    KEY_MOD_NONE = 0,

    KEY_MOD_SHIFT_LEFT = 0x0001,
    KEY_MOD_SHIFT_RIGHT = 0x0002,
    KEY_MOD_CONTROL_LEFT = 0x0004,
    KEY_MOD_CONTROL_RIGHT = 0x0008,
    KEY_MOD_ALT_LEFT = 0x0010,
    KEY_MOD_ALT_RIGHT = 0x0020,
    KEY_MOD_SUPER_LEFT = 0x0040,
    KEY_MOD_SUPER_RIGHT = 0x0080,
    KEY_MOD_CAPS_LOCK = 0x0100,
    KEY_MOD_NUM_LOCK = 0x0200,

    KEY_MOD_SHIFT = KEY_MOD_SHIFT_LEFT | KEY_MOD_SHIFT_RIGHT,
    KEY_MOD_CONTROL = KEY_MOD_CONTROL_LEFT | KEY_MOD_CONTROL_RIGHT,
    KEY_MOD_ALT = KEY_MOD_ALT_LEFT | KEY_MOD_ALT_RIGHT,
};

union EventData {
    struct KeyEventData {
        u32 scan_code;
        u32 key_code;
        u32 key_mods;
    } key;

    struct MouseEventData {
        i32 x;
        i32 y;
        u32 button;
    } mouse;

    struct WindowEventData {
        i32 x;
        i32 y;
        u16 width;
        u16 height;
    } window;
};

class Event {
public:
    Event(EventType type, EventData data): _type(type), _data(data) {}

private:
    EventType _type{};
    EventData _data{};
};

}  // namespace toki
