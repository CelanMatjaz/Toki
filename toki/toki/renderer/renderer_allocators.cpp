#include "toki/renderer/renderer_allocators.h"

#include <toki/core/core.h>

namespace toki::renderer {

static BumpAllocator s_rendererBumpAllocator(toki::GB(1));
static Allocator s_rendererPersistentAllocator(toki::GB(1));

void* RendererBumpAllocator::allocate(u64 size) {
	return s_rendererBumpAllocator.allocate(size);
}

void* RendererBumpAllocator::allocate_aligned(u64 size, u64 alignment) { 
	return s_rendererBumpAllocator.allocate_aligned(size, alignment);
}

void RendererBumpAllocator::free([[maybe_unused]]void* ptr) {}	// noop

void RendererBumpAllocator::free_aligned([[maybe_unused]]void* ptr) {}	// noop

void* RendererBumpAllocator::reallocate(void* ptr, u64 size) {
	if (ptr == nullptr) {
		return allocate(size);
	}

	return nullptr;
}

void* RendererBumpAllocator::reallocate_aligned(void* ptr, u64 size, u64 alignment) {
	void* new_ptr = allocate_aligned(size, alignment);
	if (ptr == nullptr) {
		return new_ptr;
	}
	toki::memcpy(ptr, new_ptr, size);
	return new_ptr;
}

void RendererBumpAllocator::free_to_marker(u64 marker) {
	s_rendererBumpAllocator.free_to_marker(marker);
}

void RendererBumpAllocator::clear() {
	s_rendererBumpAllocator.clear();
}

void* RendererPersistentAllocator::allocate(u64 size) {
	return s_rendererPersistentAllocator.allocate(size);
}

void* RendererPersistentAllocator::allocate_aligned(u64 size, u64 alignment) {
	return s_rendererPersistentAllocator.allocate_aligned(size, alignment);
}

void RendererPersistentAllocator::free(void* ptr) {
	s_rendererPersistentAllocator.free(ptr);
}

void RendererPersistentAllocator::free_aligned(void* ptr) {
	s_rendererPersistentAllocator.free_aligned(ptr);
}

void* RendererPersistentAllocator::reallocate(void* ptr, u64 size) {
	return s_rendererPersistentAllocator.reallocate(ptr, size);
}

void* RendererPersistentAllocator::reallocate_aligned(void* ptr, u64 size, u64 alignment) {
	void* new_ptr = allocate_aligned(size, alignment);
	if (ptr == nullptr) {
		return new_ptr;
	}
	toki::memcpy(ptr, new_ptr, size);
	return new_ptr;
}

}  // namespace toki::renderer
