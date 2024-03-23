#include "stack_allocator.h"

#include "toki/core/assert.h"
#include "toki/memory/memory.h"
#include "toki/memory/platform_memory.h"

namespace Toki {

StackAllocator::StackAllocator(uint32_t allocationSize, MemoryTag tag) :
    m_allocationSize(allocationSize),
    m_memoryBlock(allocateBlock(m_allocationSize)),
    m_memoryTag(tag) {
    addAllocationTotalForTag(m_memoryTag, allocationSize);
}

StackAllocator::~StackAllocator() {
    deallocateBlock(m_memoryBlock);
    subtractAllocationTotalForTag(m_memoryTag, m_allocationSize);
}

void* StackAllocator::alloc(uint32_t size) {
    TK_ASSERT(m_stackPointer + size >= m_allocationSize, "Cannot allocate more than {} bytes", m_allocationSize);
    m_stackPointer += size;
    addAllocationForTag(m_memoryTag, size);
    return (uint8_t*) m_memoryBlock + m_stackPointer - size;
}

void* StackAllocator::allocAligned(uint32_t size, uint32_t alignment) {
    TK_ASSERT((alignment & (alignment - 1)) == 0, "Alignment isn't a power of 2");
    uint32_t totalSize = size + alignment;

    uintptr_t raw = (uintptr_t) alloc(totalSize);

    uint32_t mask = (alignment - 1);
    uintptr_t misalignment = ((uint64_t) raw & mask);
    ptrdiff_t adjustment = alignment - misalignment;

    return (void*) raw + adjustment;
}

uint32_t StackAllocator::getPointer() const {
    return m_stackPointer;
}

void StackAllocator::freeToPointer(uint32_t pointer) {
    subtractAllocationForTag(m_memoryTag, m_stackPointer - pointer);
    m_stackPointer = pointer;
}

void StackAllocator::clear() {
    m_stackPointer = 0;
    subtractAllocationForTag(m_memoryTag, m_stackPointer);
}

}  // namespace Toki
