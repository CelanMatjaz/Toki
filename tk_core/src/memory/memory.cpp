#include "memory.h"

#if defined(TK_PLATFORM_WINDOWS)
#include <Windows.h>
#include <heapapi.h>
#endif

namespace Toki {

void* memory_allocate(uint64_t allocation_size) {
#if defined(TK_PLATFORM_WINDOWS)
    return (void*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, allocation_size);
#elif defined(TK_PLATFORM_LINUX)
    return malloc(allocation_size);
#endif
}

void memory_free(void* block) {
#if defined(TK_PLATFORM_WINDOWS)
    HeapFree(GetProcessHeap(), 0, block);
#elif defined(TK_PLATFORM_LINUX)
    free(block);
#endif
}

}  // namespace Toki
