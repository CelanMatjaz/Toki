#include "../platform_window.h"

#if defined(TK_PLATFORM_LINUX) && defined(TK_WINDOW_SYSTEM_WAYLAND)

#include <sys/socket.h>
#include <sys/un.h>

#include "../../core/assert.h"
#include "../../core/common.h"
#include "../../core/types.h"
#include "../platform.h"

namespace toki {

void window_system_initialize(const WindowSystemInit& init) {}

void window_system_shutdown() {}

NativeWindowHandle window_create(const char* title, u32 width, u32 height, const WindowInitFlags& flags) {
    return {};
}

void window_destroy(NativeWindowHandle handle) {}

Vec2<u32> window_get_dimensions(NativeWindowHandle handle) {
    return {};
}

}  // namespace toki

#endif
