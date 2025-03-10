#include "../platform_window.h"

#if defined(TK_PLATFORM_LINUX) && defined(TK_WINDOW_SYSTEM_WAYLAND)

#include <wayland-client-protocol.h>

#include "../../core/types.h"
#include "../platform.h"

namespace toki {

namespace platform {

struct wayland_data {
    wl_display* display{};
    wl_registry* registry{};
    wl_compositor* compositor{};
    wl_surface* surface{};
};

static wayland_data wayland_data{};

void window_system_initialize(const window_system_init&) {
    wayland_data.display = wl_display_connect(0);
    TK_ASSERT(wayland_data.display != nullptr, "Wayland display was not initialized");

    wayland_data.registry = wl_display_get_registry(wayland_data.display);
    TK_ASSERT(wayland_data.registry != nullptr, "Wayland registry was not initialized");
}

void window_system_shutdown() {
    wayland_data = {};
}

NATIVE_HANDLE_TYPE window_create(
    [[maybe_unused]] const char* title, [[maybe_unused]] u32 width, [[maybe_unused]] u32 height) {
    return NATIVE_HANDLE_TYPE{ .ptr = nullptr };
}

void window_destroy(NATIVE_HANDLE_TYPE handle) {
    auto _ = handle;
}

}  // namespace platform

}  // namespace toki

#endif
