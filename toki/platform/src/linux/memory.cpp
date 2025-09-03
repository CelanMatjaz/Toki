#include "toki/platform/syscalls.h"

#include <toki/core/core.h>

#include <sys/mman.h>

namespace toki {

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

}
