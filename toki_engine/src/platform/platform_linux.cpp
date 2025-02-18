#include "platform/platform.h"

#if defined(TK_PLATFORM_LINUX)

#include <sys/mman.h>

#include <csignal>

#include "core/assert.h"
#include "core/base.h"

namespace toki {

namespace platform {

void debug_break() {
    raise(SIGTRAP);
}

void* allocate(u64 size) {
    void* ptr = mmap(0, size + sizeof(u64), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    *reinterpret_cast<u64*>(ptr) = size;
    TK_ASSERT(ptr != nullptr, "Could not allocate memory");
    return reinterpret_cast<u64*>(ptr) + 1;
}

void deallocate(void* ptr) {
    u64 size = *(reinterpret_cast<u64*>(ptr) - 1);
    munmap(ptr, size);
}

}  // namespace platform

}  // namespace toki

#endif
