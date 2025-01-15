#include "platform.h"

#if defined(TK_PLATFORM_LINUX)

#include <csignal>

#include "core/assert.h"

namespace toki {

namespace platform {

void debug_break() {
    raise(SIGTRAP);
}

void* allocate(u32 size) {
    void* ptr = malloc(size);
    TK_ASSERT(ptr != nullptr, "Could not allocate memory");
    return ptr;
}

void deallocate(void* ptr) {
    free(ptr);
}

}  // namespace platform

}  // namespace toki

#endif
