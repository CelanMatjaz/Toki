#include "../platform.h"

#if defined(TK_PLATFORM_LINUX)

#include <signal.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/time.h>

#include "../../core/assert.h"
#include "../../core/common.h"
#include "linux_platform.h"

extern char** environ;

namespace toki {

void exit(i32 error) {
    TK_ASSERT_PLATFORM_ERROR(syscall(SYS_exit, error), "Error exiting program");
}

void file_write(NativeHandle handle, u32 n, const void* data) {
    TK_ASSERT_PLATFORM_ERROR(syscall(SYS_write, handle, data, n), "Error exiting program");
}

void debug_break() {
    signal(SIGTRAP, SIG_DFL);
}

void* memory_allocate(u64 size) {
    i64 ptr = syscall(SYS_mmap, 0, size + sizeof(u64), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    TK_ASSERT_PLATFORM_ERROR(ptr, "Could not allocate memory");
    *reinterpret_cast<u64*>(ptr) = size;
    return reinterpret_cast<u64*>(ptr) + 1;
}

void memory_free(void* ptr) {
    u64 size = *(reinterpret_cast<u64*>(ptr) - 1);
    TK_ASSERT_PLATFORM_ERROR(toki::syscall(SYS_munmap, ptr, size), "Could not free memory");
}

u64 time_microseconds() {
    timeval tv{};
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
}

u64 time_milliseconds() {
    timeval tv{};
    gettimeofday(&tv, NULL);
    return tv.tv_usec / 1000ULL;
}

extern char** environ;

const char* getenv(const char* var) {
    u32 length = strlen(var);
    char** env = environ;

    while (*env) {
        if (strcmp(var, *env, length) && (*env)[length] == '=') {
            return &(*env)[length + 1];
        }
        env++;
    }

    return nullptr;
}

}  // namespace toki

#endif
