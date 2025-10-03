#pragma once

#include <toki/core/core.h>

namespace toki::platform {

struct WindowSystemConfig {};

void window_system_initialize(const WindowSystemConfig& config = {});
void window_system_shutdown();
void window_system_poll_events();

enum WindowFlags : u32 {
	RESIZABLE = 1 << 0,
	SHOW_ON_CREATE = 1 << 0,
};

struct WindowConfig {
	u32 width;
	u32 height;
	const char* title;
	WindowFlags flags;
};

class Window {
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

	Vec2u32 get_dimensions() const;

	static u32 window_count() {
		return s_windowCount;
	}

private:
	inline static u32 s_windowCount = 0;

	void* m_handle;
	void* m_rendererData;
};

}  // namespace toki
