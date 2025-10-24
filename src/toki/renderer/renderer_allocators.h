#pragma once

#include <toki/core/common/type_traits.h>
#include <toki/core/types.h>

namespace toki {

struct RendererBumpAllocator {
	static void* allocate(u64 size);
	static void* allocate_aligned(u64 size, u64 alignment);
	static void free(void* ptr);				   // noop
	static void free_aligned(void* ptr);		   // noop
	static void* reallocate(void* ptr, u64 size);
	static void* reallocate_aligned(void* ptr, u64 size, u64 alignment);

	static void free_to_marker(u64 marker);
	static void reset();
};

static_assert(CIsAllocator<RendererBumpAllocator>);

struct RendererPersistentAllocator {
	static void* allocate(u64 size);
	static void* allocate_aligned(u64 size, u64 alignment);
	static void free(void* ptr);
	static void free_aligned(void* ptr);
	static void* reallocate(void* ptr, u64 size);
	static void* reallocate_aligned(void* ptr, u64 size, u64 alignment);
};

static_assert(CIsAllocator<RendererPersistentAllocator>);

}  // namespace toki
