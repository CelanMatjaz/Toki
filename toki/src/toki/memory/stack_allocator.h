#pragma once

#include <cstdint>

#include "toki/memory/memory.h"

namespace Toki {

class StackAllocator {
public:
    StackAllocator() = delete;
    explicit StackAllocator(uint32_t allocationSize, MemoryTag tag);
    ~StackAllocator();

    void* alloc(uint32_t size);
    void* allocAligned(uint32_t size, uint32_t alignment);
    uint32_t getPointer() const;
    void freeToPointer(uint32_t pointer);
    void clear();

protected:
    uint32_t m_stackPointer = 0;
    uint32_t m_allocationSize;
    void* m_memoryBlock;
    MemoryTag m_memoryTag;
};

}  // namespace Toki
