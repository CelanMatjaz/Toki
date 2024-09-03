#pragma once

#include <cstdint>

namespace Toki {

void* memory_allocate(uint64_t allocation_size);
void memory_free(void* block);

}  // namespace Toki
