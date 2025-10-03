#pragma once

#include <toki/core/common/type_traits.h>
#include <toki/core/types.h>

#if 0

#else

inline void* operator new(unsigned long, void* p) noexcept {
	return p;
}

inline void operator delete(void*, void*) noexcept {}

#endif

namespace toki {

struct DefaultAllocator {
	static void* allocate(u64 size);
	static void* allocate_aligned(u64 size, u64 alignment);
	static void free(void* ptr);
	static void free_aligned(void* ptr);
	static void* reallocate(void* ptr, u64 size);
	static void* reallocate_aligned(void* ptr, u64 size, u64 alignment);
};

static_assert(CIsAllocator<DefaultAllocator>);

struct MemoryConfig {
	u64 total_size{};
};

void memory_initialize(const MemoryConfig& config);

void memory_shutdown();

}  // namespace toki
