#pragma once

#include "../core/base.h"

namespace toki {

struct Window {
	union Handle {
		u64 uint64;
		void* ptr;
	};

	Handle handle;
	// EventHandler event_handler;
	void* internal_data;
	b8 should_close = false;
};

struct WindowCreateConfig {
	char* title;
};

Window create_window(const char* title);

void poll_events();

}  // namespace toki
