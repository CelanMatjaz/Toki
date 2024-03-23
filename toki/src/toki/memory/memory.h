#pragma once

#include <array>
#include <cstdint>

namespace Toki {

enum class MemoryTag {
    Unknown,
    Renderer,
    MEMORY_TAG_SIZE,
};

using AllocationSizes = std::array<uint64_t, (uint32_t) MemoryTag::MEMORY_TAG_SIZE>;

void addAllocationTotalForTag(MemoryTag tag, uint32_t size);
void subtractAllocationTotalForTag(MemoryTag tag, uint32_t size);

void addAllocationForTag(MemoryTag tag, uint32_t size);
void subtractAllocationForTag(MemoryTag tag, uint32_t size);

const AllocationSizes& getAllocationSizesTotal();
const AllocationSizes& getAllocationSizes();

}  // namespace Toki
