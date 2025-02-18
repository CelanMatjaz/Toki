#pragma once

#include "events/event_handler.h"

namespace toki {

namespace platform {

struct Window {
    union Handle {
        uint64_t uint64;
        void* ptr;
    };

    Handle handle;
    EventHandler event_handler;
    void* internal_data;
    b8 should_close = false;
};

struct WindowCreateConfig {
    char* title;
};

Window create_window(const char* title);

void poll_events();

}  // namespace platform

}  // namespace toki
