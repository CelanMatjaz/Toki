#include <sys/mman.h>
#include <toki/core/core.h>

#include "toki/platform/syscalls.h"

namespace toki::platform {

toki::Expected<void*, PlatformError> allocate(u64 size) {
	void* ptr = mmap(0, size + sizeof(u64), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	*reinterpret_cast<u64*>(ptr) = size;

	if (ptr == nullptr) {
		return PlatformError::MemoryAllocationFailed;
	}

	return reinterpret_cast<u64*>(ptr) + 1;
}

void free(void* ptr) {
	u64 size = *(reinterpret_cast<u64*>(ptr) - 1);
	munmap(ptr, size);
}

}  // namespace toki
