#pragma once

#include <toki/core/core.h>

namespace toki {

// struct ResourceAllocator {
// 	static void* allocate(u64 size);
// 	static void* allocate_aligned(u64 size, u64 alignment);
// 	static void free(void* ptr);
// 	static void free_aligned(void* ptr);
// 	static void* reallocate(void* ptr, u64 size);
// 	static void* reallocate_aligned(void* ptr, u64 size, u64 alignment);
// };

using ResourceAllocator = DefaultAllocator;

static_assert(CIsAllocator<ResourceAllocator>);

}  // namespace toki
