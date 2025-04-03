#include <cstdio>
#if defined(TK_PLATFORM_LINUX) && defined(TK_WINDOW_SYSTEM_WAYLAND)

#include <sys/socket.h>
#include <sys/un.h>

#include "../../core/assert.h"
#include "../../core/common.h"
#include "../../core/concepts.h"
#include "../../core/types.h"
#include "../platform.h"
#include "../platform_window.h"

#define ROUND_UP_4(n) ((n + 3) & -4)

namespace toki {

enum WaylandObjectIds : u32 {
    WL_DISPLAY,
    WL_REGISTRY,
    WL_COMPOSITOR,
    WL_SHM,
    XDG_WM_BASE,
    WL_SURFACE,
    XDG_SURFACE,
    XDG_TOPLEVEL,
    WL_SHM_POOL,

    WAYLAND_OBJECT_ID_COUNT,
};

struct WaylandState {
    static constexpr u32 wl_display = 1;
    inline static u32 next_id = wl_display + 1;
    i32 socket_fd = 0;

    static constexpr u32 REQUIRED_WL_COMPOSITOR_VERSION = 5;
    static constexpr u32 REQUIRED_XDG_WM_BASE_VERSION = 2;

    u32 ids[WAYLAND_OBJECT_ID_COUNT]{
        [WL_DISPLAY] = 1,
    };

    inline u32& operator()(WaylandObjectIds value) {
        return ids[value];
    }

    inline void create(WaylandObjectIds value) {
        ids[value] = next_id++;
    }
};

// Wayland opcodes
constexpr u16 WL_DISPLAY_GET_REGISTRY_OPCODE = 1;
constexpr u16 WL_REGISTRY_BIND_OPCODE = 0;
constexpr u16 WL_CALLBACK_DONE_OPCODE = 0;

// Event codes
constexpr u32 WL_DISPLAY_ERROR_EVENT = 0;
constexpr u32 WL_REGISTRY_GLOBAL_EVENT = 0;
constexpr u32 XDG_WM_BASE_PING_EVENT = 0;

struct {
    const char* interface_name;
    const u32 required_version;
    const WaylandObjectIds id;
} constexpr required_registry_objects[]{
    { "wl_compositor", WaylandState::REQUIRED_WL_COMPOSITOR_VERSION, WL_COMPOSITOR },
    { "xdg_wm_base", WaylandState::REQUIRED_XDG_WM_BASE_VERSION, XDG_WM_BASE },
};

static WaylandState wayland_state;

static i32 wayland_display_connect() {
    const char* xdg_runtime_dir = toki::getenv("XDG_RUNTIME_DIR");
    TK_ASSERT(xdg_runtime_dir != nullptr, "XDG_RUNTIME_DIR env variable not found");

    u64 xdg_runtime_dir_len = strlen(xdg_runtime_dir);

    struct sockaddr_un addr = { .sun_family = AF_UNIX, .sun_path = {} };
    TK_ASSERT(xdg_runtime_dir_len <= sizeof(addr.sun_path), "XDG runtime dir is too long");
    u64 socket_path_len = 0;

    toki::memcpy(xdg_runtime_dir, addr.sun_path, xdg_runtime_dir_len);
    socket_path_len += xdg_runtime_dir_len;

    addr.sun_path[socket_path_len++] = '/';

    const char* wayland_display = toki::getenv("WAYLAND_DISPLAY");
    if (wayland_display == nullptr) {
        char wayland_display_default[] = "wayland-0";
        u64 wayland_display_default_len = sizeof(wayland_display_default);

        toki::memcpy(wayland_display_default, addr.sun_path + socket_path_len, wayland_display_default_len);
        socket_path_len += wayland_display_default_len;
    } else {
        uint64_t wayland_display_len = strlen(wayland_display);
        toki::memcpy(wayland_display, addr.sun_path + socket_path_len, wayland_display_len);
        socket_path_len += wayland_display_len;
    }

    i32 fd = socket(AF_UNIX, SOCK_STREAM, 0);
    TK_ASSERT(fd != -1, "Error creating socket when initializing Wayland display");
    TK_ASSERT(
        connect(fd, (struct sockaddr*) &addr, sizeof(addr)) != -1,
        "Error connecting to socket when initializing Wayland display");

    return fd;
}

struct Header {
    u32 id;
    u16 opcode;
    u16 size;
};

template <typename T>
concept WaylandHandlerFunctionConcept = requires(T fn, Header* header, const u32* data) {
    { fn(header, data) } -> SameAsConcept<b8>;
};

static void wayland_handle_events(WaylandHandlerFunctionConcept auto handler_fn) {
    char buf[4096]{};
    i32 read_byte_count{};
    Header* header{};

    for (;;) {
        read_byte_count = recv(wayland_state.socket_fd, buf, sizeof(buf), MSG_PEEK);
        if (read_byte_count == 0) {
            break;
        }

        read_byte_count = recv(wayland_state.socket_fd, buf, sizeof(buf), 0);
        TK_ASSERT(read_byte_count != -1, "Error reading from Wayland socket");

        header = reinterpret_cast<Header*>(buf);
        while (reinterpret_cast<void*>(header) < (buf + read_byte_count)) {
            if (header->id == wayland_state(WL_DISPLAY) && header->opcode == WL_DISPLAY_ERROR_EVENT) {
                u32* data = reinterpret_cast<u32*>(header + 1);
                u32 object_id = data[0];
                u32 code = data[1];
                // u32 message_length = data[2];
                char* message = reinterpret_cast<char*>(&data[3]);
                printf("Wayland error object_id=%i, code=%i, message=%s\n", object_id, code, message);
                exit(1);
            }

            if (header->id == wayland_state(XDG_WM_BASE) && header->opcode == XDG_WM_BASE_PING_EVENT) {
                printf("halo");
            }

            if (handler_fn(header, reinterpret_cast<u32*>(header + 1))) {
                return;
            }
            header = reinterpret_cast<Header*>(reinterpret_cast<char*>(header) + header->size);
        }
    }
}

static void wayland_send(u32 id, u16 opcode, u32 data_size = 0, void* data = nullptr) {
    printf("id=%i opcode=%i\n", id, opcode);
    char msg[128]{};

    Header header{ .id = id, .opcode = opcode, .size = static_cast<u16>(sizeof(Header) + data_size) };
    *reinterpret_cast<Header*>(msg) = header;
    toki::memcpy(data, &msg[sizeof(Header)], data_size);

    i32 sent = send(wayland_state.socket_fd, msg, header.size, MSG_DONTWAIT);
    TK_ASSERT(sent == header.size, "Error sending data over Wayland socket");
}

static void wayland_send_and_wait(u32 id, u16 opcode, u32 data_size = 0, void* data = nullptr) {
    // wayland_send(id, opcode, data_size = 0, data);
    // for (bool done_callback = false; !done_callback;) {
    //     wayland_handle_events([&done_callback](Header* header, const u32* _) mutable {
    //         printf("WAITING FOR CALLBACK\n");
    //         if (header->id == WL_CALLBACK_DONE_OPCODE) {
    //             return true;
    //         }
    //
    //         return false;
    //     });
    // }
}

static void wayland_wl_display_get_registry() {
    wayland_state.create(WL_REGISTRY);
    wayland_send(wayland_state(WL_DISPLAY), WL_DISPLAY_GET_REGISTRY_OPCODE, sizeof(u32), &wayland_state(WL_REGISTRY));
}

// static void wayland_wl_compositor_create_surface() {
//     constexpr u16 WL_COMPOSITOR_CREATE_SURFACE_OPCODE = 0;
//     wayland_send(
//         wayland_state(WL_COMPOSITOR), WL_COMPOSITOR_CREATE_SURFACE_OPCODE, sizeof(u32), &wayland_state(WL_SURFACE));
// }
//
// static void wayland_xdg_wm_base_get_xdg_surface() {
//     constexpr u16 XDG_WM_BASE_REQUEST_GET_XDG_SURFACE_OPCODE = 2;
//     u32 data[2] = { wayland_state(XDG_SURFACE), wayland_state(WL_SURFACE) };
//     wayland_send(wayland_state(XDG_WM_BASE), XDG_WM_BASE_REQUEST_GET_XDG_SURFACE_OPCODE, sizeof(data), data);
// }
//
// static void wayland_xdg_surface_get_toplevel() {
//     constexpr u16 XDG_SURFACE_REQUEST_GET_TOPLEVEL_OPCODE = 1;
//     wayland_send(
//         wayland_state(XDG_SURFACE), XDG_SURFACE_REQUEST_GET_TOPLEVEL_OPCODE, sizeof(u32),
//         &wayland_state(XDG_TOPLEVEL));
// }
//
// static void wayland_wl_surface_commit() {
//     constexpr u16 XDG_SURFACE_REQUEST_COMMIT_OPCODE = 6;
//     wayland_send(wayland_state(WL_SURFACE), XDG_SURFACE_REQUEST_COMMIT_OPCODE);
// }
//
// static void wayland_xdg_surface_ack_configure(u32 configure) {
//     constexpr u16 XDG_SURFACE_ACK_CONFIGURE_OPCODE = 4;
//     wayland_send(wayland_state(XDG_SURFACE), XDG_SURFACE_ACK_CONFIGURE_OPCODE, sizeof(u32), &configure);
// }

static void wayland_init_interfaces() {
    wayland_wl_display_get_registry();

    u32 required_registry_object_count = 0;
    wayland_handle_events([&required_registry_object_count](Header* header, const u32* data) mutable {
        constexpr u32 WL_REGISTRY_DONE_CALLBACK_ID = 0;
        if (header->id == WL_REGISTRY_DONE_CALLBACK_ID) {
            return false;
        }

        if (header->id == wayland_state(WL_REGISTRY)) {
            switch (header->opcode) {
                case WL_REGISTRY_GLOBAL_EVENT: {
                    u32 name_numeric = *(data + 0);
                    u32 name_length = *(data + 1);
                    const char* interface = reinterpret_cast<const char*>((data + 2));
                    u32 rounded_up4 = ROUND_UP_4(name_length) / 4;
                    u32 version = *(data + rounded_up4 + 2);

                    u32 data[32]{};

                    for (const auto& registry_object : required_registry_objects) {
                        // printf("%b %s %s\n", toki::strcmp(interface, registry_object.interface_name), interface,
                        // registry_object.interface_name);
                        if (!toki::strcmp(interface, registry_object.interface_name)) {
                            continue;
                        }

                        TK_ASSERT(
                            version >= registry_object.required_version,
                            "%s id (%i) is not ok (required: %i)",
                            interface,
                            version,
                            registry_object.required_version);

                        wayland_state.create(registry_object.id);
                        data[0] = name_numeric;
                        data[1] = name_length;
                        toki::memcpy(interface, &data[2], name_length);
                        data[2 + rounded_up4] = registry_object.required_version;
                        data[3 + rounded_up4] = wayland_state(registry_object.id);

                        wayland_send(
                            wayland_state(WL_REGISTRY), WL_REGISTRY_BIND_OPCODE, sizeof(u32) * (4 + rounded_up4), data);

                        required_registry_object_count++;
                    }

                    if (required_registry_object_count == ARRAY_SIZE(required_registry_objects)) {
                        return true;
                    }

                    TK_LOG_INFO("Wayland interface {}", interface);

                    break;

                    return false;
                }

                default:
                    UNREACHABLE;
            }
        }

        return false;
    });

    TK_ASSERT(required_registry_object_count == ARRAY_SIZE(required_registry_objects), "Not good");
}

void window_system_initialize(const WindowSystemInit& init) {
    wayland_state.socket_fd = wayland_display_connect();

    wayland_init_interfaces();
}

void window_system_shutdown() {}

NativeWindowHandle window_create(const char* title, u32 width, u32 height, const WindowInitFlags& flags) {
    return {};
}

void window_destroy(NativeWindowHandle handle) {}

Vec2<u32> window_get_dimensions(NativeWindowHandle handle) {
    return {};
}

void window_poll_events() {}

}  // namespace toki

#endif
