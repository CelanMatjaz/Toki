#pragma once

#include <toki/core/math/vector2.h>
#include <toki/core/platform/event/event_handler.h>
#include <toki/core/types.h>

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
	u32 width;
	u32 height;
	const char* title;
	WindowFlags flags;
};

struct StaticWindowFunctions;

class Window {
	friend StaticWindowFunctions;

public:
	Window() = delete;
	Window(const WindowConfig& config);
	~Window();

	b8 should_close() const;

	void* native_handle() const {
		return m_handle;
	}

	void set_renderer_pointer(void* ptr);
	void* get_renderer_pointer() const;

	Vector2u32 get_dimensions() const;

	void clear_events() {
		m_eventQueue.clear();
	}

	bool poll_event(Event& event);

private:
	void* m_handle{};
	void* m_rendererData{};
	EventQueue m_eventQueue{};
	EventHandler m_eventHandler{};
};

}  // namespace toki
