#pragma once

#include <toki/core/types.h>

namespace toki {

struct MemoryConfig {
	u64 total_size{};
};

enum struct MemoryTag : u8 {
	UNKNOWN,

	MEMORY_TAG_COUNT
};

void memory_initialize(const MemoryConfig& config);

void memory_shutdown();

// Allocate memory of `size` bytes for UNKNOWN tag
void* memory_allocate(u64 size);

// Allocate memory of `size` bytes for tag
void* memory_allocate_with_tag(MemoryTag tag, u64 size);

// Free memory
void memory_free(void* ptr);

// Reallocate `old_block` to a block of `new_size`
void* memory_reallocate(void* old_block, u64 new_size);

// Allocate memory of `count` * sizeof(T) for tag
template <typename T>
T* memory_allocate_array_with_tag(MemoryTag tag, u64 count = 1) {
	return reinterpret_cast<T*>(memory_allocate_with_tag(tag, sizeof(T) * count));
}

// Allocate memory of `count` * sizeof(T) for UNKNOWN tag
template <typename T>
T* memory_allocate_array(u64 count = 1) {
	return memory_allocate_array_with_tag<T>(MemoryTag::UNKNOWN, count);
}

// Reallocate `old_block` to a block of `new_count` * sizeof(T)
template <typename T>
T* memory_reallocate_array(T* old_block, u64 new_count = 1) {
	return reinterpret_cast<T*>(memory_reallocate(old_block, sizeof(T) * new_count));
}

}  // namespace toki
