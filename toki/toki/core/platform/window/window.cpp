#include "toki/core/platform/window/window.h"

namespace toki {

void* Window::native_handle() const {
	return m_handle;
}

b8 Window::poll_event(Event& event) {
	return m_input.event_queue.pop(event);
}

EventQueue& Window::get_event_queue() {
	return m_input.event_queue;
}

Vector2 Window::get_mouse_position() const {
	return m_input.mouse_position;
}

Vector2 Window::get_mouse_delta() const {
	return m_input.mouse_delta;
}

Mods Window::get_mods() const {
	return m_input.mods;
}

b8 Window::is_key_down(Key key) const {
	return m_input.is_key_down(key);
}

b8 Window::is_mouse_button_pressed(MouseButton mouse_button) const {
	return m_input.is_mouse_button_down(mouse_button);
}

}  // namespace toki
