#include "memory.h"

#include "toki/core/assert.h"

namespace Toki {

static AllocationSizes allocationSizesTotal;
static AllocationSizes allocationSizesUsed;

void addAllocationTotalForTag(MemoryTag tag, uint32_t size) {
    TK_ASSERT(tag != MemoryTag::MEMORY_TAG_SIZE, "Wrong memory tag");
    TK_ASSERT(size > 0, "Size is invalid");
    allocationSizesTotal[(uint32_t) tag] += size;
}

void subtractAllocationTotalForTag(MemoryTag tag, uint32_t size) {
    TK_ASSERT(tag != MemoryTag::MEMORY_TAG_SIZE, "Wrong memory tag");
    TK_ASSERT(size > 0, "Size is invalid");
    allocationSizesTotal[(uint32_t) tag] -= size;
}

void addAllocationForTag(MemoryTag tag, uint32_t size) {
    TK_ASSERT(tag != MemoryTag::MEMORY_TAG_SIZE, "Wrong memory tag");
    TK_ASSERT(size > 0, "Size is invalid");
    allocationSizesUsed[(uint32_t) tag] += size;
}

void subtractAllocationForTag(MemoryTag tag, uint32_t size) {
    TK_ASSERT(tag != MemoryTag::MEMORY_TAG_SIZE, "Wrong memory tag");
    TK_ASSERT(size > 0, "Size is invalid");
    allocationSizesUsed[(uint32_t) tag] -= size;
}

const AllocationSizes& getAllocationSizes() {
    return allocationSizesUsed;
}

const AllocationSizes& getAllocationSizesTotal() {
    return allocationSizesTotal;
}

}  // namespace Toki
