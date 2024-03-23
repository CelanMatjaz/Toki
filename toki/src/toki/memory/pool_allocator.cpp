#include "pool_allocator.h"

#include "platform_memory.h"
#include "toki/core/assert.h"

namespace Toki {

PoolAllocator::PoolAllocator(uint32_t elementStride, uint32_t elementCount, MemoryTag tag) :
    m_allocationSize(elementStride + sizeof(void*) * elementCount),
    m_memoryBlock(allocateBlock(m_allocationSize)),
    m_elementStride(elementStride),
    m_elementCount(elementCount),
    m_memoryTag(tag) {
    addAllocationForTag(m_memoryTag, m_allocationSize);

    m_nextFreeArray = (void**) ((char*) m_memoryBlock + m_elementStride * m_elementCount);

    for (uint32_t i = 0; i < m_elementCount; ++i) {
        m_nextFreeArray[i] = ((char*) m_memoryBlock + m_elementStride * i);
    }
}

PoolAllocator::~PoolAllocator() {
    subtractAllocationForTag(m_memoryTag, m_allocationSize);
}

void* PoolAllocator::getNext() {
    TK_ASSERT(m_allocatedElementCount < m_elementCount, "Cannot allocate more than {} elements", m_elementCount);
    TK_ASSERT(m_nextFreeArray[m_nextFreeIndex] != nullptr, "Unexpected nullptr");

    void* element = m_nextFreeArray[m_nextFreeIndex];

    return nullptr;
}

}  // namespace Toki
