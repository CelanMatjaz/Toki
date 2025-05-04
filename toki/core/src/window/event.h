#pragma once

namespace toki {

enum struct WindowEvent : u32 {
	KEY_PRESS,
	KEY_RELEASE,
	KEY_REPEAT,

	MOUSE_PRESS,
	MOUSE_RELEASE,
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

union EventData {
	u64 value;

	struct {
		u8 key_code;
		u8 scan_code;
		u8 mods;
	} key;

	struct {
		i16 x;
		i16 y;
		u8 button_code;
	} mouse_button;

	struct {
		i32 x;
		i32 y;
	} window_move, mouse_move, mouse_scroll;

	struct {
		u32 x;
		u32 y;
	} window_resize;
};

struct Event {
	static Event create(WindowEvent event_type, EventData data = {}) {
		return Event{ .data = data, .event_type = event_type };
	}

	bool operator==(const WindowEvent& type) const {
		return event_type == type;
	}

	operator WindowEvent() {
		return event_type;
	}

	EventData data;
	b32 is_handled{ false };
	WindowEvent event_type;
};

}  // namespace toki
