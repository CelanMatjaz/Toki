#include "toki/core/platform/memory.h"

#include <sys/mman.h>
#include <toki/core/common/assert.h>

namespace toki {

void* allocate(u64 size) {
	void* result = ::mmap(0, size + sizeof(u64), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	TK_ASSERT(result != MAP_FAILED, "Memory allocation failed");
	*reinterpret_cast<i64*>(result) = size;
	return reinterpret_cast<i64*>(result) + 1;
}

void free(void* ptr) {
	i64* size = reinterpret_cast<i64*>(ptr) - 1;
	i64 result = ::munmap(size, *size);
	TK_ASSERT(result != -1, "Memory deallocation failed");
}

}  // namespace toki
