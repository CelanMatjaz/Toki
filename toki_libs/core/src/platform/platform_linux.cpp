#include <wayland-client-core.h>

#include "platform.h"

#if defined(TK_PLATFORM_LINUX)

#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>

#include "core/assert.h"

namespace toki {

namespace platform {

#define MAX_EVENT_PER_LOOP_COUNT 256

void debug_break() {
    signal(SIGTRAP, SIG_DFL);
}

void* memory_allocate(u64 size) {
    void* ptr = mmap(0, size + sizeof(u64), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    *reinterpret_cast<u64*>(ptr) = size;
    TK_ASSERT(ptr != nullptr, "Could not allocate memory");
    return reinterpret_cast<u64*>(ptr) + 1;
}

void memory_free(void* ptr) {
    u64 size = *(reinterpret_cast<u64*>(ptr) - 1);
    munmap(ptr, size);
}

u64 get_time_microseconds() {
    timeval tv{};
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
}

u64 get_time_milliseconds() {
    timeval tv{};
    gettimeofday(&tv, NULL);
    return tv.tv_usec / 1000ULL;
}

void file_delete(PATH_TYPE path) {
    syscall(SYS_unlink, path);
}

b8 file_exists(PATH_TYPE path) {
    i64 result = syscall(SYS_access, path, F_OK);
    return result == 0;
}

b8 directory_exists(PATH_TYPE path) {
    struct stat st{};
    u64 result = syscall(SYS_stat, path, &st);

    if (result == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }

    return false;
}

Stream::Stream(PATH_TYPE path, u32 flags) {
    i32 internal_flags = 0;

    if (flags & STREAM_FLAGS::TRUNC) {
        internal_flags |= O_TRUNC;
    }

    if (flags & STREAM_FLAGS::ATE) {
        internal_flags |= O_APPEND;
    }

    if (flags & STREAM_FLAGS::OUTPUT && flags & STREAM_FLAGS::INPUT) {
        internal_flags |= O_RDWR;
    } else if (flags & STREAM_FLAGS::INPUT) {
        internal_flags |= O_RDONLY;
    } else if (flags & STREAM_FLAGS::OUTPUT) {
        internal_flags |= O_WRONLY;
    }

    i64 fd = syscall(SYS_openat, AT_FDCWD, path, internal_flags);
    if (fd < 0) {
        TK_ASSERT(false, "File %lli not opened", fd);
    }

    m_NativeHandle.i64 = fd;
}

Stream::~Stream() {
    close();
}

void Stream::seek(STREAM_OFFSET_TYPE offset) {
    static_assert(sizeof(STREAM_OFFSET_TYPE) == 8, "Below function assumes that offset type is int64");

    m_Offset = syscall(SYS_lseek, m_NativeHandle.i64, offset, SEEK_SET);
    TK_ASSERT(m_Offset != -1, "Could not set file pointer");
}

Stream::STREAM_OFFSET_TYPE Stream::tell() {
    m_Offset = syscall(SYS_lseek, m_NativeHandle.i64, 0, SEEK_CUR);
    TK_ASSERT(m_Offset != -1, "Could not get file pointer");
    return m_Offset;
}

void Stream::close() {
    if (m_NativeHandle.i64 != 0) {
        syscall(SYS_close, m_NativeHandle.i64);
    }
}

void initialize_wayland() {
    wayland_display = wl_display_connect(0);
    TK_ASSERT(wayland_display != nullptr, "Wayland display was not initialized");
}

void shutdown_wayland() {
    TK_ASSERT(wayland_display != nullptr, "Cannot shutdown uninitialized wayland display");
    wl_display_disconnect(wayland_display);
    wayland_display = {};
}

NATIVE_HANDLE_TYPE window_create(const char* title, u32 width, u32 height) {
    return NATIVE_HANDLE_TYPE{ .ptr = nullptr };
}

void window_destroy(NATIVE_HANDLE_TYPE handle) {}

}  // namespace platform

}  // namespace toki

#endif
