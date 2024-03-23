#pragma once

#include <cstdint>

namespace Toki {

void* allocateBlock(uint64_t allocationSize);
void deallocateBlock(void* memoryBlock);

}  // namespace Toki
