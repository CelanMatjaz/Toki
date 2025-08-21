#include "platform/linux_platform.h"

#include <GLFW/glfw3.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <ctime>

#include "../core/assert.h"
#include "../core/common.h"
#include "../core/types.h"
#include "../memory/memory.h"
#include "../string/string.h"
#include "../window/window.h"
#include "defines.h"
#include "platform.h"
#include "print.h"

extern char** environ;

namespace toki {

static_assert(sizeof(long) == 8);

struct PlatformState {
	struct Wrapper {
		Window window;
		b32 allocated;
	} windows[PLATFORM_MAX_WINDOW_COUNT + 1];

	Window* find_free() {
		for (u32 i = 0; i < PLATFORM_MAX_WINDOW_COUNT; i++) {
			if (!windows[i].allocated) {
				windows[i].allocated = true;
				return &windows[i].window;
			}
		}

		return nullptr;
	}

	Window* get_stub() {
		TK_ASSERT(!windows[PLATFORM_MAX_WINDOW_COUNT].allocated, "Stub already allocated");
		windows[PLATFORM_MAX_WINDOW_COUNT].allocated = true;
		return &windows[PLATFORM_MAX_WINDOW_COUNT].window;
	}

	void destroy(Window* window) {
		Wrapper* wrapper = reinterpret_cast<Wrapper*>(window);
		wrapper->window.~Window();
		wrapper->allocated = false;
	}
};

static PlatformState* s_platform_state{};

void platform_initialize() {
	s_platform_state = memory_allocate_array<PlatformState>();

#if defined(TK_WINDOW_SYSTEM_GLFW)
	glfwInit();
#endif
}

void platform_shutdown() {
#if defined(TK_WINDOW_SYSTEM_GLFW)
	glfwTerminate();
#endif

	memory_free(s_platform_state);
	s_platform_state = nullptr;
}

WeakRef<Window> window_create(const Window::Config& config) {
	Window* first_free = s_platform_state->find_free();
	TK_ASSERT(first_free != nullptr, "Too many windows already created:", PLATFORM_MAX_WINDOW_COUNT);
	return emplace<Window>(first_free, toki::move(config));
}

WeakRef<Window> window_create_stub(const Window::Config& config) {
	Window* first_free = s_platform_state->get_stub();
	TK_ASSERT(first_free != nullptr, "Too many windows already created:", PLATFORM_MAX_WINDOW_COUNT);
	return emplace<Window>(first_free, toki::move(config));
}

void window_destroy(Window* window) {
	s_platform_state->destroy(window);
}

const char* getenv(const char* var) {
	return ::getenv(var);
}

void exit(i32 error) {
	exit(error);
}

i64 read(NativeHandle handle, char* buf, u64 count) {
	return ::read(handle, buf, count);
}

i64 write(NativeHandle handle, const char* buf, u64 count) {
	return ::write(handle, buf, count);
}

NativeHandle open(const char* pathname, u32 flags) {
	return ::open(pathname, flags);
}

i64 close(NativeHandle handle) {
	return ::close(handle);
}

void* allocate(u64 size) {
	void* result = ::mmap(0, size + sizeof(u64), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	TK_ASSERT(result != MAP_FAILED, "Memory allocation failed");
	*reinterpret_cast<i64*>(result) = size;
	return reinterpret_cast<i64*>(result) + 1;
}

void deallocate(void* ptr) {
	i64* size = reinterpret_cast<i64*>(ptr) - 1;
	i64 result = ::munmap(size, *size);
	TK_ASSERT(result != -1, "Memory deallocation failed");
}

u64 time_nanoseconds() {
	timespec tspec{};

	i64 result = ::clock_gettime(CLOCK_REALTIME, &tspec);
	TK_ASSERT(result != -1, "Error getting time");
	return static_cast<u64>(tspec.tv_sec) * 1000000000ULL + static_cast<u64>(tspec.tv_nsec);
}

NativeHandle socket_create() {
	i64 result = ::socket(AF_UNIX, SOCK_STREAM, 0);
	TK_ASSERT(result != -1, "Error creating socket");
	return result;
}

i64 socket_connect(NativeHandle handle, void* addr, u64 addr_size) {
	i64 result = ::connect(handle, (struct sockaddr*) addr, addr_size);
	TK_ASSERT(result != -1, "Error connecting to socket");
	return result;
}

static u32 map_file_stream_flags(u32 flags) {
	u32 flags_out = 0;

	if (flags & FILE_RDWR) {
		flags_out |= O_RDWR;
	} else if (flags & FILE_WRITE) {
		flags_out |= O_WRONLY;
	} else {
		flags_out |= O_RDONLY;
	}

	if (flags & FILE_APPEND) {
		flags_out |= O_APPEND;
	}

	if (flags & FILE_TRUNC) {
		flags_out |= O_TRUNC;
	}

	return flags_out;
}

FileStream::FileStream(const char* path, u32 flags) {
	m_handle = open(path, map_file_stream_flags(flags));
}

FileStream::~FileStream() {}

u64 FileStream::write(const void* data, u64 size) {
	return ::write(m_handle, data, size);
}

u64 FileStream::read(void* data, u64 size) {
	return ::read(m_handle, data, size);
}

u64 FileStream::tell() {
	return ::lseek(m_handle, 0, SEEK_CUR);
}

u64 FileStream::seek(u64 pos) {
	return ::lseek(m_handle, pos, SEEK_SET);
}

Socket::Socket(): m_handle(socket_create()) {}

Socket::~Socket() {
	::close(m_handle);
}

void Socket::connect(const StringView& path) {
	struct sockaddr_un addr = { .sun_family = AF_UNIX };
	toki::memcpy(path.data(), addr.sun_path, path.size());
	socket_connect(m_handle, (struct sockaddr*) &addr, sizeof(addr));

	print_args(m_handle.handle, addr.sun_path, addr.sun_family, path.size());
}

void Socket::close() {
	::close(m_handle);
}

u32 Socket::send(const void* msg, u32 size) {
	i64 result = ::send(m_handle, msg, size, 0);
	TK_ASSERT(result != -1, "Error sending message", errno);
	return result;
}

u32 Socket::recv(void* msg, u32 size) {
	i64 result = ::recv(m_handle, msg, size, 0);
	TK_ASSERT(result != -1, "Error receiving message");
	return result;
}

}  // namespace toki
