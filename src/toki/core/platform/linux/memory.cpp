#include <sys/mman.h>
#include <toki/core/core.h>
#include <toki/core/platform/syscalls.h>

namespace toki {

toki::Expected<void*, TokiError> allocate(u64 size) {
	void* ptr = mmap(0, size + sizeof(u64), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	if (ptr == nullptr) {
		return TokiError::MEMORY_ALLOCATION_FAILED;
	}

	*reinterpret_cast<u64*>(ptr) = size;
	return reinterpret_cast<u64*>(ptr) + 1;
}

void free(void* ptr) {
	u64 size = *(reinterpret_cast<u64*>(ptr) - 1);
	munmap(ptr, size);
}

}  // namespace toki
