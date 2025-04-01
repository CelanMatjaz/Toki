#include "../platform.h"

#if defined(TK_PLATFORM_LINUX)

#include "../../core/common.h"
#include "../../core/concepts.h"
#include <signal.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "../../core/assert.h"
#include "../../core/common.h"

extern char** environ;

namespace toki {

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

const char* getenv(const char* var) {
    u32 length = strlen(var);
    char** env = environ;

    while (*env) {
        if (strcmp(var, *env, length) && (*env)[length] == '=') {
            return env[length + 1];
        }
        env++;
    }

    return nullptr;
}

}  // namespace toki

#endif
