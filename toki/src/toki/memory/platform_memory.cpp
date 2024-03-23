#include "platform_memory.h"

#include "platform.h"
#include "toki/core/assert.h"

#ifdef TK_PLATFORM_WINDOWS
#include <heapapi.h>
#endif

namespace Toki {

void* allocateBlock(uint64_t allocationSize) {
#ifdef TK_PLATFORM_WINDOWS
    void* memoryBlock = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, allocationSize);

    auto code = GetExceptionCode();
    TK_ASSERT(code != STATUS_NO_MEMORY, "Lack of available memory or heap corruption");

    return memoryBlock;
#endif
}

void deallocateBlock(void* memoryBlock) {
#ifdef TK_PLATFORM_WINDOWS
    bool result = HeapFree(GetProcessHeap(), 0, memoryBlock);
    TK_ASSERT(result != 0, "Error deallocating head memory");
#endif
}

}  // namespace Toki
