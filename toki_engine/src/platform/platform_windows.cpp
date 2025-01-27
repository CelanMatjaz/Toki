#include "platform.h"

#if defined(TK_PLATFORM_WINDOWS)

#include "core/assert.h"

#define NOMINMAX
#include <Windows.h>

namespace toki {

namespace platform {

void debug_break() {
    DebugBreak();
}

void* allocate(u32 size) {
    void* ptr = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    TK_ASSERT(ptr != nullptr, "Could not allocate memory");
    return ptr;
}

void deallocate(void* ptr) {
    bool result = HeapFree(GetProcessHeap(), 0, ptr);
    TK_ASSERT(result != 0, "Error deallocating memory");
}

}  // namespace platform

}  // namespace toki

#endif
