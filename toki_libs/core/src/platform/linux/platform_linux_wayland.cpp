#include <cstdio>
#include <print>
#if defined(TK_PLATFORM_LINUX) && defined(TK_WINDOW_SYSTEM_WAYLAND)
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "../../core/assert.h"
#include "../../core/common.h"
#include "../../core/logging.h"
#include "../../core/types.h"
#include "../platform.h"
#include "../platform_window.h"
#include "platform_linux_wayland.h"

#define ROUND_UP_4(n) ((n + 3) & -4)

namespace toki {

// Event codes
constexpr u32 WL_DISPLAY_ERROR_EVENT = 0;
constexpr u16 WL_CALLBACK_DONE_EVENT = 0;
constexpr u32 WL_REGISTRY_GLOBAL_EVENT = 0;
constexpr u32 XDG_WM_BASE_PING_EVENT = 0;
constexpr u32 XDG_SURFACE_CONFIGURE_EVENT = 0;

static void wayland_handle_events(WaylandHandlerFunctionConcept auto handler_fn);

static WaylandState wayland_state{};

struct xdg_toplevel : wl_struct {
    static constexpr u16 XDG_TOPLEVEL_DESTROY_OPCODE = 0;
    static constexpr u16 XDG_TOPLEVEL_SET_TITLE_OPCODE = 2;

    inline void destroy() {
        wayland_send(id, XDG_TOPLEVEL_DESTROY_OPCODE);
    }

    inline void set_title(const char* title) {
        u32 title_length = strlen(title);
        u32 data[30]{};
        TK_ASSERT(title_length < sizeof(data) - 3 * sizeof(u32), "Title is too long");
        data[0] = title_length + 1;
        toki::memcpy(title, &data[1], data[0]);
        wayland_send(id, XDG_TOPLEVEL_SET_TITLE_OPCODE, sizeof(u32) + ROUND_UP_4(data[0]), data);
    }
};

struct xdg_surface : wl_struct {
    b32 acknowledged = false;

    static constexpr u16 XDG_SURFACE_DESTROY_OPCODE = 0;
    static constexpr u16 XDG_SURFACE_GET_TOPLEVEL_OPCODE = 1;
    static constexpr u16 XDG_SURFACE_SET_WINDOW_GEOMETRY_OPCODE = 3;
    static constexpr u16 XDG_SURFACE_ACK_CONFIGURE_OPCODE = 4;

    inline void destroy() {
        wayland_send(id, XDG_SURFACE_DESTROY_OPCODE);
    }

    inline xdg_toplevel get_toplevel() {
        u32 toplevel_id = wayland_state.create();
        wayland_send(id, XDG_SURFACE_GET_TOPLEVEL_OPCODE, sizeof(u32), &toplevel_id);
        return { toplevel_id };
    }

    inline void set_geometry(int x, int y, int width, int height) {
        int data[4] = { x, y, width, height };
        wayland_send(id, XDG_SURFACE_SET_WINDOW_GEOMETRY_OPCODE, sizeof(data), data);
    }

    inline void ack_configure(u32 serial) {
        wayland_send(id, XDG_SURFACE_ACK_CONFIGURE_OPCODE, sizeof(serial), &serial);
    }
};

struct wl_buffer : wl_struct {};

struct wl_surface : wl_struct {
    static constexpr u16 WL_SURFACE_DESTROY_OPCODE = 0;
    static constexpr u16 WL_SURFACE_ATTACH_OPCODE = 1;
    static constexpr u16 WL_SURFACE_DAMAGE_OPCODE = 2;
    static constexpr u16 WL_SURFACE_COMMIT_OPCODE = 6;

    inline void destroy() {
        wayland_send(id, WL_SURFACE_DESTROY_OPCODE);
    }

    inline void attach(wl_buffer wl_buffer, i32 x, i32 y) {
        u32 data[3]{ wl_buffer, static_cast<u32>(x), static_cast<u32>(y) };
        wayland_send(id, WL_SURFACE_ATTACH_OPCODE, sizeof(data), data);
    }

    inline void damage(i32 x, i32 y, i32 width, i32 height) {
        i32 data[4]{ x, y, width, height };
        wayland_send(id, WL_SURFACE_DAMAGE_OPCODE, sizeof(data), data);
    }

    inline void commit() {
        wayland_send(id, WL_SURFACE_COMMIT_OPCODE);
    }
};

struct xdg_wm_base : wl_struct {
    static constexpr u16 XDG_WM_BASE_GET_XDG_SURFACE_OPCODE = 2;
    static constexpr u16 XDG_WM_BASE_PONG_OPCODE = 3;

    inline xdg_surface get_xdg_surface(wl_surface wl_surface) {
        u32 data[2] = { wayland_state.create(), wl_surface };
        wayland_send(id, XDG_WM_BASE_GET_XDG_SURFACE_OPCODE, sizeof(data), data);
        return { { data[0] } };
    }
};

struct wl_compositor : wl_struct {
    static constexpr u16 WL_COMPOSITOR_CREATE_SURFACE_OPCODE = 0;

    wl_surface create_surface() {
        u32 surface_id = wayland_state.create();
        wayland_send(wayland_state.get(WL_COMPOSITOR), WL_COMPOSITOR_CREATE_SURFACE_OPCODE, sizeof(u32), &surface_id);
        return { surface_id };
    }
};

struct wl_shm_pool : wl_struct {
    wl_buffer create_buffer(i32 offset, i32 width, i32 height, i32 stride, u32 format) {
        i32 data[6]{
            static_cast<i32>(wayland_state.create()), offset, width, height, stride, static_cast<i32>(format),
        };
        wayland_send(id, 0, sizeof(data), data);

        return { static_cast<u32>(data[0]) };
    }
};

struct wl_shm : wl_struct {
    wl_shm_pool create_pool() {
        Header header{ id, 0, 16 };
        u32 msg[4]{ 0, 0, wayland_state.create(), wayland_state.shm_pool_size };
        *reinterpret_cast<Header*>(msg) = header;

        u32 msg_size = sizeof(msg);

        char buf[CMSG_SPACE(sizeof(wayland_state.shm_fd))] = "";

        struct iovec io = { .iov_base = msg, .iov_len = msg_size };
        struct msghdr socket_msg = {
            .msg_iov = &io,
            .msg_iovlen = 1,
            .msg_control = buf,
            .msg_controllen = sizeof(buf),
        };

        struct cmsghdr* cmsg = CMSG_FIRSTHDR(&socket_msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(wayland_state.shm_fd));

        *((int*) CMSG_DATA(cmsg)) = wayland_state.shm_fd;
        socket_msg.msg_controllen = CMSG_SPACE(sizeof(wayland_state.shm_fd));

        if (sendmsg(wayland_state.socket_fd, &socket_msg, 0) == -1)
            exit(errno);

        return { msg[2] };
    }
};

struct wl_registry : wl_struct {
    static constexpr u16 WL_REGISTRY_BIND_OPCODE = 0;

    void bind(WaylandGlobalIds id, u32 name_numeric, u32 name_length, const char* interface, u32 version) {
        u32 data[32]{};
        u32 rounded_up4 = ROUND_UP_4(name_length) / 4;

        wayland_state.create(id);
        data[0] = name_numeric;
        data[1] = name_length;
        toki::memcpy(interface, &data[2], name_length);
        data[2 + rounded_up4] = version;
        data[3 + rounded_up4] = wayland_state.get(id);

        wayland_send(wayland_state.get(WL_REGISTRY), WL_REGISTRY_BIND_OPCODE, sizeof(u32) * (4 + rounded_up4), data);
    }
};

struct wl_callback : wl_struct {};

struct wl_display : wl_struct {
    static constexpr u16 WL_DISPLAY_SYNC_OPCODE = 0;
    static constexpr u16 WL_DISPLAY_GET_REGISTRY_OPCODE = 1;

    inline wl_callback sync() {
        const u32 callback_id = wayland_state.create();
        wayland_send(wayland_state.get(WL_DISPLAY), WL_DISPLAY_SYNC_OPCODE, sizeof(u32), &callback_id);
        return wl_callback{ callback_id };
    }

    inline wl_registry get_registry() {
        wayland_state.create(WL_REGISTRY);
        const u32 registry_id = wayland_state.get(WL_REGISTRY);
        wayland_send(wayland_state.get(WL_DISPLAY), WL_DISPLAY_GET_REGISTRY_OPCODE, sizeof(u32), &registry_id);
        return wl_registry{ registry_id };
    }
};

static void wayland_handle_events(WaylandHandlerFunctionConcept auto handler_fn) {
    char buf[4096]{};
    i32 read_byte_count{};
    Header* header{};
    b8 handled = false;

    for (; !handled;) {
        read_byte_count = recv(wayland_state.socket_fd, buf, sizeof(buf), 0);
        TK_ASSERT(read_byte_count != -1, "Error reading from Wayland socket");

        header = reinterpret_cast<Header*>(buf);
        while (reinterpret_cast<void*>(header) < (buf + read_byte_count)) {
            // Handle error
            if (header->id == wayland_state.get(WL_DISPLAY) && header->opcode == WL_DISPLAY_ERROR_EVENT) {
                u32* data = reinterpret_cast<u32*>(header + 1);
                u32 object_id = data[0];
                u32 code = data[1];
                // u32 message_length = data[2];
                char* message = reinterpret_cast<char*>(&data[3]);
                printf("Wayland error object_id=%i, code=%i, %s\n", object_id, code, message);
                TK_UNREACHABLE();
            }

            // Handle sync
            if (header->id == wayland_state.get(WL_CALLBACK) && header->opcode == WL_CALLBACK_DONE_EVENT) {
                return;
            }

            // Handle XDG_WM_BASE ping/pong
            if (header->id == wayland_state.get(XDG_WM_BASE) && header->opcode == XDG_WM_BASE_PING_EVENT) {
                printf("halo");
            }

            // Call provided handler
            if (handler_fn(header, reinterpret_cast<u32*>(header + 1))) {
                handled = true;
            }

            header = reinterpret_cast<Header*>(reinterpret_cast<char*>(header) + header->size);
        }
    }
}

void wayland_sync() {
    wayland_state.globals.wl_display->sync();
    b8 synced = false;
    while (!synced) {
        wayland_handle_events([&synced](Header* header, const u32*) {
            if (header->opcode == WL_CALLBACK_DONE_EVENT) {
                synced = true;
                return true;
            }

            return false;
        });
    }
}

void wayland_send(const u32 id, const u16 opcode, const u32 data_size, const void* data) {
    printf("sending: id=%i opcode=%i\n", id, opcode);
    char msg[128]{};

    Header header{ .id = id, .opcode = opcode, .size = static_cast<u16>(sizeof(Header) + data_size) };
    *reinterpret_cast<Header*>(msg) = header;
    toki::memcpy(data, &msg[sizeof(Header)], data_size);

    i32 sent = send(wayland_state.socket_fd, msg, header.size, MSG_DONTWAIT);
    TK_ASSERT(sent == header.size, "Error sending data over Wayland socket");
}

void WaylandState::display_connect() {
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
        u64 wayland_display_len = strlen(wayland_display);
        toki::memcpy(wayland_display, addr.sun_path + socket_path_len, wayland_display_len);
        socket_path_len += wayland_display_len;
    }

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    TK_ASSERT(socket_fd != -1, "Error creating socket when initializing Wayland display");
    TK_ASSERT(
        connect(socket_fd, (struct sockaddr*) &addr, sizeof(addr)) != -1,
        "Error connecting to socket when initializing Wayland display");
}

void WaylandState::display_disconnect() {
    close(wayland_state.socket_fd);
}

void WaylandState::init_interfaces() {
    *wayland_state.globals.wl_registry = wayland_state.globals.wl_display->get_registry();

    u32 required_registry_object_count = 0;
    wayland_handle_events([&required_registry_object_count](Header* header, const u32* data) mutable {
        if (header->id == wayland_state.get(WL_REGISTRY) && header->opcode == WL_REGISTRY_GLOBAL_EVENT) {
            u32 name_numeric = *(data + 0);
            u32 name_length = *(data + 1);
            const char* interface = reinterpret_cast<const char*>((data + 2));
            u32 rounded_up4 = ROUND_UP_4(name_length) / 4;
            u32 version = *(data + rounded_up4 + 2);

            // printf("checking interface %s\n", interface);

            for (const auto& registry_object : required_registry_objects) {
                if (!toki::strcmp(interface, registry_object.interface_name)) {
                    continue;
                }

                TK_ASSERT(
                    version >= registry_object.required_version,
                    "%s version (%i) is not satisfied (required: %i)",
                    interface,
                    version,
                    registry_object.required_version);

                wayland_state.globals.wl_registry->bind(
                    registry_object.id, name_numeric, name_length, interface, registry_object.required_version);

                required_registry_object_count++;
                TK_LOG_INFO("Found required Wayland interface {}", interface);
                break;
            }

            if (required_registry_object_count == ARRAY_SIZE(required_registry_objects)) {
                return true;
            }
        }

        return false;
    });

    TK_ASSERT(required_registry_object_count == ARRAY_SIZE(required_registry_objects), "Not good");
}

void window_system_initialize(const WindowSystemInit& init) {
    wayland_state.display_connect();
    wayland_state.init_interfaces();

#if defined(TK_DEBUG) && defined(WAYLAND_TEST_BUFFER)

#endif
}

void window_system_shutdown() {
    wayland_state.display_disconnect();
}

NativeWindowHandle window_create(const char* title, u32 width, u32 height, const WindowInitFlags& flags) {
    wl_surface wl_surface = wayland_state.globals.wl_compositor->create_surface();
    xdg_surface xdg_surface = wayland_state.globals.xdg_wm_base->get_xdg_surface(wl_surface);
    xdg_toplevel xdg_toplevel = xdg_surface.get_toplevel();

    xdg_surface.set_geometry(0, 0, width, height);
    xdg_toplevel.set_title(title);
    wl_surface.commit();

    wayland_state.globals.print();
    printf("wl_surface     %i\n", wl_surface.id);
    printf("xdg_surface    %i\n", xdg_surface.id);
    printf("xdg_toplevel   %i\n", xdg_toplevel.id);
    wayland_handle_events([&xdg_surface](Header* header, const u32* data) {
        printf("event: object_id=%i, opcode=%i\n", header->id, header->opcode);
        if (header->id == xdg_surface.id && xdg_surface::XDG_SURFACE_ACK_CONFIGURE_OPCODE) {
            xdg_surface.acknowledged = true;

            return true;
        }

        return false;
    });

    TK_ASSERT(xdg_surface.acknowledged, "XDG_SURFACE configure was not acknowledged")

    wl_surface.commit();

    // wayland_sync();

    wayland_state.create_shared_memory_file();
    wl_shm_pool wl_shm_pool = wayland_state.globals.wl_shm->create_pool();
    wl_buffer wl_buffer = wl_shm_pool.create_buffer(
        0, wayland_state.buffer_width, wayland_state.buffer_height, wayland_state.buffer_stride, 1);

    uint32_t* pixels = (uint32_t*) wayland_state.shm_pool_data;
    for (uint32_t i = 0; i < wayland_state.buffer_width * wayland_state.buffer_height; i++) {
        pixels[i] = (u32) -1;
    }

    wl_surface.attach(wl_buffer, 0, 0);
    wl_surface.damage(0, 0, 400, 400);

    printf("window created %i\n", wl_surface.id);

    // while (true) {
    //     window_poll_events();
    // }

    return { .wl = { wayland_state.globals.wl_display->id, wl_surface, xdg_surface, xdg_toplevel } };
}

void window_destroy(NativeWindowHandle handle) {
    xdg_toplevel{ handle.wl.xdg_toplevel }.destroy();
    xdg_surface{ { handle.wl.xdg_surface } }.destroy();
}

Vec2<u32> window_get_dimensions(NativeWindowHandle handle) {
    return {};
}

void window_poll_events() {
    wayland_handle_events([](Header* header, const u32*) {
        printf("event: object_id=%i, opcode=%i\n", header->id, header->opcode);

        return false;
    });
}

}  // namespace toki

#endif
