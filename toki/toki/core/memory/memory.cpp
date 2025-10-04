#include "toki/core/memory/memory.h"

#include <toki/core/math/math.h>
#include <toki/core/memory/allocator.h>
#include <toki/core/utils/memory.h>
#include <toki/platform/syscalls.h>

namespace toki {

static Allocator g_allocator;

void memory_initialize(const MemoryConfig& config) {
	g_allocator = toki::move(Allocator(config.total_size));
}

void memory_shutdown() {
	g_allocator = {};
}

void* DefaultAllocator::allocate(u64 size) {
	return g_allocator.allocate(size);
}

void* DefaultAllocator::allocate_aligned(u64 size, u64 alignment) {
	return g_allocator.allocate_aligned(size, alignment);
}

void DefaultAllocator::free(void* ptr) {
	g_allocator.free(ptr);
}

void DefaultAllocator::free_aligned(void* ptr) {
	g_allocator.free_aligned(ptr);
}

void* DefaultAllocator::reallocate(void* ptr, u64 size) {
	return g_allocator.reallocate(ptr, size);
}

void* DefaultAllocator::reallocate_aligned(void* ptr, u64 size, u64 alignment) {
	void* new_ptr = allocate_aligned(size, alignment);
	if (ptr == nullptr) {
		return new_ptr;
	}
	toki::memcpy(new_ptr, ptr, size);
	return new_ptr;
}

}  // namespace toki
