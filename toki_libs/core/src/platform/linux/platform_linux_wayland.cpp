#include "../platform_window.h"

#if defined(TK_PLATFORM_LINUX) && defined(TK_WINDOW_SYSTEM_WAYLAND)

#include <sys/socket.h>
#include <sys/un.h>

#include "../../core/assert.h"
#include "../../core/common.h"
#include "../../core/types.h"
#include "../platform.h"

namespace toki {

struct WaylandState {
    static constexpr u32 wl_display = 1;
    inline static u32 next_id = wl_display + 1;
    i32 socket = 0;

    u32 wl_registry = 0;
    u32 wl_shm = 0;
    u32 wl_compositor = 0;
    u32 xdg_wm_base = 0;
    u32 wl_surface = 0;
    u32 xdg_surface = 0;
    u32 xdg_toplevel = 0;
    u32 wl_shm_pool = 0;
};

static WaylandState wayland_state;

static int wayland_display_connect() {
    const char* xdg_runtime_dir = toki::getenv("XDG_RUNTIME_DIR");
    if (xdg_runtime_dir == NULL)
        return EINVAL;

    u64 xdg_runtime_dir_len = strlen(xdg_runtime_dir);

    struct sockaddr_un addr = { .sun_family = AF_UNIX, .sun_path = {} };
    TK_ASSERT(xdg_runtime_dir_len <= sizeof(addr.sun_path), "XDG runtime dir is too long");
    u64 socket_path_len = 0;

    toki::memcpy(xdg_runtime_dir, addr.sun_path, xdg_runtime_dir_len);
    socket_path_len += xdg_runtime_dir_len;

    addr.sun_path[socket_path_len++] = '/';

    const char* wayland_display = getenv("WAYLAND_DISPLAY");
    if (wayland_display == NULL) {
        char wayland_display_default[] = "wayland-0";
        u64 wayland_display_default_len = sizeof(wayland_display_default);

        toki::memcpy(wayland_display_default, addr.sun_path + socket_path_len, wayland_display_default_len);
        socket_path_len += wayland_display_default_len;
    } else {
        uint64_t wayland_display_len = strlen(wayland_display);
        toki::memcpy(wayland_display, addr.sun_path + socket_path_len, wayland_display_len);
        socket_path_len += wayland_display_len;
    }

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    TK_ASSERT(fd == -1, "Error creating socket when initializing Wayland display");
    TK_ASSERT(
        connect(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1,
        "Error connecting to socket when initializing Wayland display");

    return fd;
}

struct Header {
    uint32_t id;
    uint16_t opcode;
    uint16_t size;
};

static void wayland_send(u32 id, u16 opcode, u32 data_size = 0, void* data = nullptr) {
    char msg[128]{};

    Header header{ .id = id, .opcode = opcode, .size = static_cast<u16>(sizeof(Header) + data_size) };
    *reinterpret_cast<Header*>(msg) = header;
    toki::memcpy(data, &msg[sizeof(Header)], data_size);

    i32 sent = send(wayland_state.socket, msg, header.size, MSG_DONTWAIT);
    TK_ASSERT(sent == header.size, "Error sending data over Wayland socket");
}

static void wayland_wl_get_registry() {
    constexpr uint16_t DISPLAY_GET_REGISTRY_OPCODE = 1;
    wayland_state.wl_registry = WaylandState::next_id++;
    wayland_send(wayland_state.wl_display, DISPLAY_GET_REGISTRY_OPCODE, sizeof(uint32_t), &wayland_state.wl_registry);
}

void wayland_wl_compositor_create_surface() {
    constexpr uint32_t WL_COMPOSITOR_CREATE_SURFACE_OPCODE = 0;
    wayland_state.wl_surface = WaylandState::next_id++;
    wayland_send(
        wayland_state.wl_compositor, WL_COMPOSITOR_CREATE_SURFACE_OPCODE, sizeof(uint32_t), &wayland_state.wl_surface);
}

void wayland_xdg_wm_base_get_xdg_surface() {
    constexpr uint32_t XDG_WM_BASE_REQUEST_GET_XDG_SURFACE_OPCODE = 2;
    wayland_state.xdg_surface = WaylandState::next_id++;
    uint32_t data[2] = { wayland_state.xdg_surface, wayland_state.wl_surface };
    wayland_send(wayland_state.xdg_wm_base, XDG_WM_BASE_REQUEST_GET_XDG_SURFACE_OPCODE, sizeof(data), data);
}

void wayland_xdg_surface_get_toplevel() {
    constexpr uint32_t XDG_SURFACE_REQUEST_GET_TOPLEVEL_OPCODE = 1;
    wayland_state.xdg_toplevel = WaylandState::next_id++;
    wayland_send(
        wayland_state.xdg_surface,
        XDG_SURFACE_REQUEST_GET_TOPLEVEL_OPCODE,
        sizeof(uint32_t),
        &wayland_state.xdg_toplevel);
}

void wayland_wl_surface_commit() {
    constexpr uint32_t XDG_SURFACE_REQUEST_COMMIT_OPCODE = 6;
    wayland_send(wayland_state.wl_surface, XDG_SURFACE_REQUEST_COMMIT_OPCODE);
}

void wayland_wl_display_get_registry() {
    constexpr uint32_t WL_DISPLAY_GET_REGISTRY_OPCODE = 1;
    wayland_state.xdg_toplevel = WaylandState::next_id++;
    wayland_send(
        wayland_state.wl_display, WL_DISPLAY_GET_REGISTRY_OPCODE, sizeof(uint32_t), &wayland_state.wl_registry);
};

void wayland_xdg_surface_ack_configure(uint32_t configure) {
    constexpr uint16_t XDG_SURFACE_ACK_CONFIGURE_OPCODE = 4;
    wayland_send(wayland_state.xdg_surface, XDG_SURFACE_ACK_CONFIGURE_OPCODE, sizeof(uint32_t), &configure);
}

void window_system_initialize(const WindowSystemInit& init) {
    wayland_state.socket = wayland_display_connect();
}

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
