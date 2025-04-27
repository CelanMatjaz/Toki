#include "platform/linux_platform.h"

#include "platform.h"

namespace toki {

namespace pt {

// Syscall registers in order
// a, D, S, d, r10, r8, r9

u64 last_error = 0;

inline i64 check_for_error(i64 result) {
	if (result < 0) {
		last_error = -result;
		return -1;
	}
	return result;
}

i64 exit(i32 error) {
	i64 result{};
	asm("syscall" : "=r"(result) : "a"(SYS_EXIT), "D"(error));
	return check_for_error(result);
}

i64 read(Handle handle, char* buf, u64 count) {
	i64 result{};
	asm("syscall" : "=r"(result) : "a"(SYS_READ), "D"(handle.handle), "S"(buf), "d"(count));
	return check_for_error(result);
}

i64 write(Handle handle, const char* buf, u64 count) {
	i64 result{};
	asm("syscall" : "=r"(result) : "a"(SYS_WRITE), "D"(handle.handle), "S"(buf), "d"(count));
	return check_for_error(result);
}

i64 open(const char* pathname, i32 flags) {
	i64 result{};
	asm("syscall" : "=r"(result) : "a"(SYS_OPEN), "D"(pathname), "S"(flags | FILE_CREAT), "d"(0777));
	return check_for_error(result);
}

i64 close(Handle handle) {
	i64 result{};
	asm("syscall" : "=r"(result) : "a"(SYS_CLOSE), "D"(handle.handle));
	return check_for_error(result);
}

void* allocate(u64 size) {
	i64 ptr{};

	register i64 r10 asm("r10") = static_cast<i64>(SharingTypes::MAP_PRIVATE);
	register i64 r8 asm("r8") = static_cast<i64>(-1);
	register i64 r9 asm("r9") = static_cast<i64>(0);
	asm("syscall"
		: "=r"(ptr)
		: "a"(SYS_OPEN), "D"(0), "S"(size + sizeof(u64)), "d"(PROT_RDWR), "r"(r10), "r"(r8), "r"(r9));
	TK_ASSERT(check_for_error(ptr) != -1, "Memory allocation failed");
	return reinterpret_cast<void*>(ptr);
}

void deallocate(void* ptr) {
	u64 size = *reinterpret_cast<u64*>(ptr);
	i64 result{};
	asm("syscall" : "=r"(result) : "a"(SYS_MUNMAP), "D"(ptr), "S"(size));
	TK_ASSERT(check_for_error(result) != -1, "Memory deallocation failed");
}

}  // namespace pt

}  // namespace toki
