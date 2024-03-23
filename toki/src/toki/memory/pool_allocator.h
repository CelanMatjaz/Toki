#pragma once

#include <cstdint>

#include "toki/memory/memory.h"

namespace Toki {

class PoolAllocator {
    PoolAllocator() = delete;
    explicit PoolAllocator(uint32_t elementStride, uint32_t elementCount, MemoryTag tag);
    ~PoolAllocator();

    void* getNext();
    void freeElement(void* element);

private:
    uint32_t m_allocationSize;
    void* m_memoryBlock;
    MemoryTag m_memoryTag;
    uint32_t m_elementStride;
    uint32_t m_elementCount;
    void** m_nextFreeArray = nullptr;
    uint32_t m_nextFreeIndex = 0;

    uint32_t m_allocatedElementCount = 0;
};

}  // namespace Toki
