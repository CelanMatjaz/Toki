#pragma once

#include <toki/core/core.h>

namespace toki {

struct WindowSystemConfig {};

void window_system_initialize(const toki::WindowSystemConfig& config = {});
void window_system_shutdown();
void window_system_poll_events();

enum WindowConfigFlags : u32 {
	RESIZABLE = 1 << 0,
	SHOW_ON_CREATE = 1 << 0,
};

struct WindowConfig {
	u32 width;
	u32 height;
	const char* title;
	WindowConfigFlags flags;
};

class Window {
public:
	Window(const toki::WindowConfig& config);
	~Window();

	b8 should_close() const;

	void* native_handle() const {
		return m_handle;
	}

	static u32 window_count() {
		return s_windowCount;
	}

private:
	inline static u32 s_windowCount = 0;

	void* m_handle;
};

}  // namespace toki
