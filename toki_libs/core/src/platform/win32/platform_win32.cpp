#include "../platform.h"

#if defined(TK_PLATFORM_WINDOWS)

#include <Shlwapi.h>
#include <Windows.h>

#include "core/assert.h"

namespace toki {

namespace platform {

#define MAX_EVENT_PER_LOOP_COUNT 256

void debug_break() {
    DebugBreak();
}

void* memory_allocate(u64 size) {
    void* ptr = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    return ptr;
}

void memory_free(void* ptr) {
    bool _ = HeapFree(GetProcessHeap(), 0, ptr);
}

u64 get_time() {
    FILETIME ft{};
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER ull{};
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return (ull.QuadPart - 116444736000000000ULL);
}

u64 get_time_microseconds() {
    return get_time() / 10ULL;
}

u64 get_time_milliseconds() {
    return get_time() / 10000ULL;
}

}  // namespace platform

}  // namespace toki

#endif
