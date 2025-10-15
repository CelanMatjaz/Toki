#pragma once

#include <toki/core/platform/event/mods.hpp>

namespace toki {

enum class EventType : u8 {
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

enum class MouseButton : u8 {
	NONE,
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,

	MOUSE_BUTTON_COUNT
};

enum class Key : u8 {
	KEY_COUNT
};

struct KeyEventData {
	u32 scan;
	u8 key;
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
