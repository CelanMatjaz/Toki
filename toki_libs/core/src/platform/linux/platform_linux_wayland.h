#pragma once

#if defined(TK_PLATFORM_LINUX) && defined(TK_WINDOW_SYSTEM_WAYLAND)

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#include "../../core/types.h"

namespace toki {

namespace platform {

void initialize_wayland();
void shutdown_wayland();

struct wayland_data {
    wl_display* display{};
    wl_registry* registry{};
    wl_compositor* compositor{};
    wl_surface* surface{};
};

inline wayland_data wayland_data{};

void registry_handler(void* data, struct wl_registry* registry, u32 id, const char* interface, u32 version);

}  // namespace platform

}  // namespace toki

#endif
