#include "toki/core/memory/memory.h"

#include <toki/core/math/math.h>
#include <toki/core/memory/allocator.h>
#include <toki/core/platform/syscalls.h>
#include <toki/core/utils/memory.h>

void* operator new([[maybe_unused]] unsigned long size, void* p) noexcept {
	return p;
}

void operator delete(void*, void*) {}

namespace toki {

static Allocator g_allocator;

void memory_initialize(const MemoryConfig& config) {
	g_allocator					= toki::move(Allocator(config.total_size));
	DefaultAllocator::allocator = &g_allocator;
}

void memory_shutdown() {
	g_allocator = {};
}

void* DefaultAllocator::allocate(u64 size) {
	return DefaultAllocator::allocator->allocate(size);
}

void* DefaultAllocator::allocate_aligned(u64 size, u64 alignment) {
	return DefaultAllocator::allocator->allocate_aligned(size, alignment);
}

void DefaultAllocator::free(void* ptr) {
	DefaultAllocator::allocator->free(ptr);
}

void DefaultAllocator::free_aligned(void* ptr) {
	DefaultAllocator::allocator->free_aligned(ptr);
}

void* DefaultAllocator::reallocate(void* ptr, u64 size) {
	return DefaultAllocator::allocator->reallocate(ptr, size);
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
