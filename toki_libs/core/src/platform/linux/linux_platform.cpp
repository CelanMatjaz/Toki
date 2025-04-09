#include "linux_platform.h"

#include <sys/mman.h>
#include <sys/syscall.h>

#include <csignal>

#include "../../core/assert.h"
#include "../../core/string.h"

namespace toki {

#if defined (TK_WINDOW_SYSTEM_WAYLAND)
#endif

void platform_initialize(const PlatformInit& init) {
#if defined (TK_WINDOW_SYSTEM_WAYLAND)

#endif
}

void platform_shutdown() {}

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

void exit(i32 error) {
    TK_ASSERT_PLATFORM_ERROR(syscall(SYS_exit, error), "Error exiting program");
}

void debug_break() {
    signal(SIGTRAP, SIG_DFL);
}

extern char** environ;

const char* getenv(const char* var) {
    u32 length = toki::strlen(var);
    char** env = environ;

    while (*env) {
        if (toki::strcmp(var, *env, length) && (*env)[length] == '=') {
            return &(*env)[length + 1];
        }
        env++;
    }

    return nullptr;
}

}  // namespace toki
