#pragma once

#include <toki/core.h>

#include "events/event.h"

namespace toki {

struct EventHandler {
    void handle(Event&) {}
};

struct Window {
public:
    void handle_events(EventHandler& handler);

    inline b8 should_close() const {
        return mShouldClose;
    }

    NativeWindowHandle mNativeHandle;
    b32 mShouldClose{ false };
};

inline Window sWindows[TK_MAX_WINDOW_COUNT];
inline u32 sWindowCount = 0;

}  // namespace toki
