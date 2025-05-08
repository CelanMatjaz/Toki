#pragma once

#include "../core/types.h"
#include "../math/types.h"
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

	Vec2<u32> dimensions() const {
		return m_dimensions;
	};

	static void poll_events();

	void* m_renderer_data{};

private:
	NativeWindow m_native_window{};
	Vec2<u32> m_dimensions{};
};

}  // namespace toki
