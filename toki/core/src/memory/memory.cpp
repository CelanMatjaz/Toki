#include "memory.h"

#include "allocator.h"
#include "core/assert.h"
#include "math/math.h"
#include "string/string.h"

namespace toki {

struct MemoryState {
	Allocator allocator;
	u64 total_allocated;
	u64 tagged_allocations[static_cast<u32>(MemoryTag::MEMORY_TAG_COUNT)]{};
};

struct Header {
	u64 total_size : 56;
	MemoryTag tag : 8;
};

static MemoryState* s_memory_state{};

void memory_initialize(const MemoryConfig& config) {
	Allocator allocator(config.total_size);
	s_memory_state = reinterpret_cast<MemoryState*>(allocator.allocate(sizeof(MemoryState)));
	s_memory_state->allocator = move(allocator);
}

void memory_shutdown() {
	s_memory_state->allocator.free(s_memory_state);
}

static void* _allocate_with_tag(MemoryTag tag, u64 size_without_header) {
	TK_ASSERT(size_without_header > 0, "Cannot allocate a block of 0 size");
	u64 total_size = (size_without_header + sizeof(Header) + 8) & -8;  // Align everything to 8
	Header* header = reinterpret_cast<Header*>(s_memory_state->allocator.allocate(total_size));
	header->total_size = total_size;
	header->tag = tag;

	s_memory_state->tagged_allocations[static_cast<u32>(header->tag)] += total_size;
	s_memory_state->total_allocated += total_size;

	return header + 1;
}

static void _deallocate_with_tag(void* ptr) {
	Header* header = reinterpret_cast<Header*>(ptr) - 1;
	s_memory_state->tagged_allocations[static_cast<u32>(header->tag)] -= header->total_size;
}

void* memory_allocate(u64 size) {
	return _allocate_with_tag(MemoryTag::UNKNOWN, size);
}

void* memory_allocate_with_tag(MemoryTag tag, u64 size) {
	return _allocate_with_tag(tag, size);
}

void memory_free(void* ptr) {
	_deallocate_with_tag(ptr);
}

void* memory_reallocate(void* old_block, u64 new_size) {
	if (old_block == nullptr) {
		return memory_allocate(new_size);
	}

	Header* original_header = reinterpret_cast<Header*>(old_block) - 1;
	void* new_ptr = memory_allocate(new_size);
	u64 copy_size = max(original_header->total_size, new_size);
	toki::memcpy(original_header + 1, new_ptr, copy_size);
	memory_free(old_block);
	return new_ptr;
}

}  // namespace toki
