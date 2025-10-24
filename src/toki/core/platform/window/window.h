#pragma once

#include <toki/core/math/vector2.h>
#include <toki/core/types.h>

#include "toki/core/platform/input/input.h"

namespace toki {

struct WindowSystemConfig {};

void window_system_initialize(const WindowSystemConfig& config = {});
void window_system_shutdown();
void window_system_poll_events();

enum WindowFlags : u32 {
	WINDOW_FLAG_RESIZABLE = 1 << 0,
	WINDOW_FLAG_SHOW_ON_CREATE = 1 << 0,
};

struct WindowConfig {
	Vector2u32 dimensions{};
	Vector2u32 min_dimensions{};
	const char* title;
	WindowFlags flags;
};

struct StaticWindowFunctions;

class Window {
	friend class Engine;
	friend StaticWindowFunctions;

public:
	Window() = delete;
	Window(const WindowConfig& config);
	~Window();

	Vector2u32 get_dimensions() const;
	Vector2 get_mouse_position() const;
	Vector2 get_mouse_delta() const;
	Mods get_mods() const;
	b8 is_key_down(Key key) const;
	b8 is_mouse_button_pressed(MouseButton mouse_button) const;

	void* native_handle() const;
	void register_listener(void* listener, EventFunction fn);
	void unregister_listener(void* listener);

private:
	b8 should_close() const;
	b8 poll_event(Event& event);
	EventQueue& get_event_queue();

private:
	void* m_handle{};
	Input m_input{};
	Vector2u32 m_currentSize{};
};

}  // namespace toki
