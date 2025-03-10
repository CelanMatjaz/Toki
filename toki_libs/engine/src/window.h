#pragma once

#include <toki/core.h>

#include "events/event.h"

namespace toki {

constexpr u32 MAX_ENGINE_WINDOW_COUNT = 1;

#if defined(TK_PLATFORM_WINDOWS)
LRESULT window_proc(HWND handle, u32 msg, WPARAM w_param, LPARAM l_param);
#endif

struct EventHandler {
    void handle(Event&) {}
};

struct Window {
public:
    struct Config {
        const char* title;
        u32 width, height;
    };

    void create(const Config& config);
    void destroy();

    void handle_events(EventHandler& handler);

    inline b8 should_close() const {
        return m_should_close;
    }

private:
    platform::NATIVE_HANDLE_TYPE m_native_handle;
    b32 m_should_close{ false };
};

inline static u32 s_window_count = 0;
inline static Window s_windows[MAX_ENGINE_WINDOW_COUNT];

}  // namespace toki
