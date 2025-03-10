#include "platform_linux_wayland.h"

#if defined(TK_PLATFORM_LINUX) && defined(TK_WINDOW_SYSTEM_WAYLAND)

#include <wayland-client-protocol.h>

#include "../../core/types.h"
#include "../platform.h"

namespace toki {

namespace platform {

void window_system_initialize() {
    wayland_data.display = wl_display_connect(0);
    TK_ASSERT(wayland_data.display != nullptr, "Wayland display was not initialized");

    wayland_data.registry = wl_display_get_registry(wayland_data.display);
    TK_ASSERT(wayland_data.registry != nullptr, "Wayland registry was not initialized");
}

void window_system_shutdown() {
    TK_ASSERT(wayland_data.display != nullptr, "Cannot shutdown uninitialized wayland display");
    wl_display_disconnect(wayland_data.display);

    wayland_data = {};
}

NATIVE_HANDLE_TYPE window_create(const char* title, u32 width, u32 height) {
    return NATIVE_HANDLE_TYPE{ .ptr = nullptr };
}

void window_destroy(NATIVE_HANDLE_TYPE handle) {}

}  // namespace platform

}  // namespace toki

#endif
