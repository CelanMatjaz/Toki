#pragma once

#include <toki/core/containers/bitset.h>
#include <toki/core/math/vector2.h>
#include <toki/core/platform/input/event_defines.h>
#include <toki/core/platform/input/event_handler.h>

namespace toki {

constexpr const u32 MOUSE_BUTTON_COUNT = static_cast<u64>(MouseButton::MOUSE_BUTTON_COUNT);
constexpr const u32 KEYBOARD_BUTTON_COUNT = static_cast<u64>(Key::KEY_COUNT);

struct Input {
	Vector2 mouse_position;
	Vector2 mouse_delta;
	Bitset<MOUSE_BUTTON_COUNT> mouse_buttons;
	Bitset<KEYBOARD_BUTTON_COUNT> keys;
	Mods mods;

	EventQueue event_queue;
	EventHandler event_handler;

	u32 state_index;

	b8 is_key_down(Key) const;
	b8 is_mouse_button_down(MouseButton key) const;
};

inline b8 Input::is_key_down(Key key) const {
	return keys.at(static_cast<u8>(key));
}

inline b8 Input::is_mouse_button_down(MouseButton key) const {
	return mouse_buttons.at(static_cast<u8>(key));
}

}  // namespace toki
