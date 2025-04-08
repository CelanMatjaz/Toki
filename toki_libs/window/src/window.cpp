#include "window.h"

#if defined(TK_PLATFORM_WINDOWS)

#elif defined(TK_PLATFORM_LINUX)

#if defined(TK_WINDOW_SYSTEM_WAYLAND)
#include "wayland.h"
extern toki::WaylandState wayland_state;
static void _initialize() {
    wayland_state.display_connect();
    wayland_state.init_interfaces();
}
static void _shutdown() {
    wayland_state.display_disconnect();
}
#endif

#endif

namespace toki {

void WindowSystem::initialize() {
    _initialize();
}

void WindowSystem::shutdown() {
    _shutdown();
}

void WindowSystem::poll_events() {}

}  // namespace toki
