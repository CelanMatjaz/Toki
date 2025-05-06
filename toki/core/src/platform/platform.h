#pragma once

#include "../containers/weak_ref.h"
#include "../core/types.h"
#include "../string/string_view.h"
#include "../window/window.h"
#include "defines.h"

namespace toki {

void platform_initialize();

void platform_shutdown();

WeakRef<Window> window_create(const Window::Config& config);

void window_destroy(Window* window);

constexpr u64 STD_IN = 0;
constexpr u64 STD_OUT = 1;
constexpr u64 STD_ERR = 2;

extern u64 last_error;

inline u64 get_last_error() {
	return last_error;
}

const char* getenv(const char* var);

void exit(i32 error);

i64 read(NativeHandle handle, char* buf, u64 count);

i64 write(NativeHandle handle, const char* buf, u64 count);

NativeHandle open(const char* pathname, u32 flags);

i64 close(NativeHandle handle);

void* allocate(u64 size);

void deallocate(void* ptr);

u64 time_nanoseconds();

NativeHandle socket_create();

i64 socket_connect(NativeHandle handle);

class FileStream {
public:
	FileStream() = delete;
	FileStream(const char* path, u32 flags = FILE_READ);
	~FileStream();

	u64 write(const void* data, u64 size);
	u64 read(void* data, u64 size);

	u64 tell();
	u64 seek(u64 pos);

private:
	NativeHandle m_handle{};
};

class Socket {
public:
	Socket();
	~Socket();

	void connect(const StringView& path);
	void close();
	u32 send(const void* msg, u32 size);
	u32 recv(void* msg, u32 size);

private:
	NativeHandle m_handle;
};

}  // namespace toki
