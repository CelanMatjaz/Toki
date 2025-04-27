#pragma once

#include "core/concepts.h"
#include "print.h"
#include "socket.h"
#include "types/types.h"

namespace toki {

struct wl_struct {
	u32 id;

	operator u32() const {
		return id;
	}

	b8 valid() {
		return id != 0;
	}
};

struct wl_display;
struct wl_registry;
struct wl_callback;
struct wl_compositor;
struct xdg_wm_base;

struct WaylandGlobals {
	struct wl_display* wl_display;
	struct wl_registry* wl_registry;
	struct wl_callback* wl_callback;
	struct wl_compositor* wl_compositor;
	struct wl_shm* wl_shm;
	struct wl_seat* wl_seat;
	struct xdg_wm_base* xdg_wm_base;

	void print() {
		toki::println("wl_callback:   %i", *reinterpret_cast<u32*>(wl_callback));
		toki::println("wl_display:    %i\n", *reinterpret_cast<u32*>(wl_display));
		toki::println("wl_registry:   %i\n", *reinterpret_cast<u32*>(wl_registry));
		toki::println("wl_seat:       %i\n", *reinterpret_cast<u32*>(wl_seat));
		toki::println("wl_compositor: %i\n", *reinterpret_cast<u32*>(wl_compositor));
		toki::println("wl_shm:        %i\n", *reinterpret_cast<u32*>(wl_shm));
		toki::println("xxg_wm_base:   %i\n", *reinterpret_cast<u32*>(xdg_wm_base));
	}
};

enum WaylandGlobalIds {
	WL_DISPLAY,
	WL_REGISTRY,
	WL_CALLBACK,
	WL_COMPOSITOR,
	WL_SHM,
	WL_SEAT,
	XDG_WM_BASE,

	WAYLAND_GLOBAL_ID_COUNT
};

inline static constexpr u32 REQUIRED_WL_COMPOSITOR_VERSION = 6;
inline static constexpr u32 REQUIRED_XDG_WM_BASE_VERSION = 6;
inline static constexpr u32 REQUIRED_WL_SEAT_VERSION = 9;

void wayland_send(const u32 id, const u16 opcode, const u32 data_size = 0, const void* data = nullptr);

struct {
	const char* interface_name;
	const u32 required_version;
	const WaylandGlobalIds id;
} inline constexpr required_registry_objects[]{
	{ "wl_compositor", REQUIRED_WL_COMPOSITOR_VERSION, WL_COMPOSITOR },
	{ "xdg_wm_base", REQUIRED_XDG_WM_BASE_VERSION, XDG_WM_BASE },
	{ "wl_seat", REQUIRED_WL_SEAT_VERSION, WL_SEAT },
	{ "wl_shm", 1, WL_SHM },
};

class WaylandState {
public:
	// ↓↓↓↓ Nasty ↓↓↓↓
	WaylandGlobals globals = {
		reinterpret_cast<struct wl_display*>(&global_ids[WL_DISPLAY]),
		reinterpret_cast<struct wl_registry*>(&global_ids[WL_REGISTRY]),
		reinterpret_cast<struct wl_callback*>(&global_ids[WL_CALLBACK]),
		reinterpret_cast<struct wl_compositor*>(&global_ids[WL_COMPOSITOR]),
		reinterpret_cast<struct wl_shm*>(&global_ids[WL_SHM]),
		reinterpret_cast<struct wl_seat*>(&global_ids[WL_SEAT]),
		reinterpret_cast<struct xdg_wm_base*>(&global_ids[XDG_WM_BASE]),
	};

	void display_connect();
	void display_disconnect();
	void init_interfaces();

	inline const u32 get(WaylandGlobalIds value) const {
		return global_ids[value].id;
	}

	inline u32 create() {
		return next_id++;
	}

	inline void create(WaylandGlobalIds value) {
		global_ids[value].id = create();
	}

	Socket socket;

	// u32 buffer_width = 400;
	// u32 buffer_height = 400;
	// u32 buffer_stride = buffer_width * 4;
	// u32 shm_pool_size = buffer_height * buffer_stride;
	i64 shm{};
	char* shm_data;

	void create_shared_memory_file(u32 size);

private:
	wl_struct global_ids[WAYLAND_GLOBAL_ID_COUNT]{
		[WL_DISPLAY] = { 1 },
	};
	static constexpr u32 wl_display = 1;
	inline static u32 next_id = wl_display + 1;
};

struct Header {
	u32 id;
	u16 opcode;
	u16 size;
};

template <typename T>
concept WaylandHandlerFunctionConcept = requires(T fn, Header* header, const u32* data) {
	{ fn(header, data) } -> SameAsConcept<b8>;
};

}  // namespace toki
