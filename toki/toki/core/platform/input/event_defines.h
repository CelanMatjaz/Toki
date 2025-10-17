#pragma once

#include <toki/core/platform/input/mods.hpp>

namespace toki {

enum struct EventType : u8 {
	NONE,

	KEY_PRESS,
	KEY_RELEASE,
	KEY_REPEAT,

	MOUSE_PRESS,
	MOUSE_RELEASE,
	MOUSE_REPEAT,
	MOUSE_MOVE,
	MOUSE_SCROLL,

	WINDOW_MOVE,
	WINDOW_RESIZE,
	WINDOW_MAXIMIZE,
	WINDOW_MINIMIZE,
	WINDOW_RESTORE,
	WINDOW_FOCUS,
	WINDOW_BLUR,
	WINDOW_CLOSE,
	WINDOW_ENTER,
	WINDOW_LEAVE,

	EVENT_COUNT
};

enum struct MouseButton : u8 {
	NONE,
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,

	MOUSE_BUTTON_COUNT
};

// clang-format off
enum struct Key : u8 {
    NONE,

    SPACE,
    APOSTROPHE,
    COMMA,
    MINUS,
    PERIOD,
    SLASH,

    NUM_0,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,

    SEMICOLON,
    EQUAL,

    A, B, C, D, E, F, G, H, I, J,
    K, L, M, N, O, P, Q, R, S, T,
    U, V, W, X, Y, Z,

    LEFT_BRACKET,
    BACKSLASH,
    RIGHT_BRACKET,
    GRAVE_ACCENT,
    WORLD_1,         // non-US #1
    WORLD_2,         // non-US #2

    ESCAPE,
    ENTER,
    TAB,
    BACKSPACE,
    INSERT,
    DELETE,
    RIGHT,
    LEFT,
    DOWN,
    UP,
    PAGE_UP,
    PAGE_DOWN,
    HOME,
    END,

    CAPS_LOCK,
    SCROLL_LOCK,
    NUM_LOCK,
    PRINT_SCREEN,
    PAUSE,

    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,
    F11, F12, F13, F14, F15, F16, F17, F18, F19, F20,
    F21, F22, F23, F24, F25,

    KP_0, KP_1, KP_2, KP_3, KP_4, KP_5, KP_6, KP_7, KP_8, KP_9,
    KP_DECIMAL,
    KP_DIVIDE,
    KP_MULTIPLY,
    KP_SUBTRACT,
    KP_ADD,
    KP_ENTER,
    KP_EQUAL,

    LEFT_SHIFT,
    LEFT_CONTROL,
    LEFT_ALT,
    LEFT_SUPER,
    RIGHT_SHIFT,
    RIGHT_CONTROL,
    RIGHT_ALT,
    RIGHT_SUPER,
    MENU,

	KEY_COUNT
};
// clang-format on

struct KeyEventData {
	u32 scan;
	Key key;
};

struct MouseEventData {
	i16 x, y;
	u8 button;
};

struct WindowEventData {
	i16 x, y;
};

union EventData {
	KeyEventData key;
	MouseEventData mouse;
	WindowEventData window;
};

static_assert(sizeof(EventData) <= 8);

}  // namespace toki
