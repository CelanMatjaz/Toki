#include "../platform.h"
#include "core/common.h"
#include "core/concepts.h"

#if defined(TK_PLATFORM_LINUX)

#include <signal.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "core/assert.h"

namespace toki {

namespace platform {

void debug_break() {
    signal(SIGTRAP, SIG_DFL);
}

void unreachable() {
#if (defined(__GNUC__) || defined(__clang__)) && __cplusplus >= 202302L
    __builtin_unreachable();
#else
#error "unreachable compiler function not found, only Clang and GCC supported"
#endif
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

template <typename... Args>
constexpr void print(const char* fmt, Args&&... args) {
    u32 total_length = strlen(fmt);

    // ([&](auto&& arg) {
    //     if constexpr (isSameValue<decltype(arg), int>) {
    //     }
    //
    //     else {
    //     }
    // }(args)...);
}

}  // namespace platform

}  // namespace toki

#endif
