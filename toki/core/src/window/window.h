#pragma once

#include "../core/types.h"
#include "../platform/defines.h"
#include "event_handler.h"

namespace toki {

class Window {
public:
	struct Flags {};

	struct Config {
		const char* title;
		u32 width;
		u32 height;
		Flags flags;
	};

	Window() = delete;
	Window(const Config& config);
	~Window();

	inline const NativeWindow get_native_window() const {
		return m_native_window;
	}

	WindowEventHandler handler;

	static void poll_events();

private:
	NativeWindow m_native_window{};
};

}  // namespace toki
