#include "platform/linux_platform.h"

#include "../containers/dynamic_array.h"
#include "../window/window.h"
#include "containers/basic_ref.h"
#include "containers/static_array.h"
#include "core/base.h"
#include "core/common.h"
#include "core/concepts.h"
#include "defines.h"
#include "memory/allocator.h"
#include "platform.h"
#include "print.h"
#include "window/event_handler.h"

namespace toki {

namespace pt {

static_assert(sizeof(long) == 8);

struct PlatformState {
	Window windows[PLATFORM_MAX_WINDOW_COUNT]{};
};

static PlatformState* s_platform_state;

void initialize(Allocator& allocator) {
	emplace<PlatformState>(allocator.allocate(sizeof(PlatformState)));
}

void shutdown(Allocator& allocator) {
	allocator.free(s_platform_state);
	s_platform_state = nullptr;
}

u64 last_error = 0;

inline static i64 check_for_error(i64 result) {
	if (result < 0) {
		last_error = -result;
		return -1;
	}
	return result;
}

// Syscall registers in order
// a, D, S, d, r10, r8, r9

template <typename T>
concept SyscallArgumentConcept = IsConvertibleConcept<T, i64>;

template <typename... Args>
	requires(SyscallArgumentConcept<Args> && ...) && (sizeof...(Args) <= 6)
[[nodiscard]] i64 syscall(u64 _syscall, Args... args) {
	i64 casted_args[sizeof...(Args)] = { ((i64) (args))... };

	i64 ret{};

	if constexpr (sizeof...(Args) == 1) {
		asm volatile("syscall"
					 : "=a"(ret)
					 : "a"(_syscall), "D"(static_cast<i64>(casted_args[0]))
					 : "rcx", "r11", "memory");
	}

	else if constexpr (sizeof...(Args) == 2) {
		asm volatile("syscall"
					 : "=a"(ret)
					 : "a"(_syscall), "D"(static_cast<i64>(casted_args[0])), "S"(static_cast<i64>(casted_args[1]))
					 : "rcx", "r11", "memory");
	}

	else if constexpr (sizeof...(Args) == 3) {
		asm volatile("syscall"
					 : "=a"(ret)
					 : "a"(_syscall),
					   "D"(static_cast<i64>(casted_args[0])),
					   "S"(static_cast<i64>(casted_args[1])),
					   "d"(static_cast<i64>(casted_args[2]))
					 : "rcx", "r11", "memory");
	}

	else if constexpr (sizeof...(Args) == 4) {
		register i64 r10 asm("r10") = static_cast<i64>(casted_args[3]);
		asm volatile("syscall"
					 : "=a"(ret)
					 : "a"(_syscall),
					   "D"(static_cast<i64>(casted_args[0])),
					   "S"(static_cast<i64>(casted_args[1])),
					   "d"(static_cast<i64>(casted_args[2])),
					   "r"(r10)
					 : "rcx", "r11", "memory");
	}

	else if constexpr (sizeof...(Args) == 5) {
		register i64 r10 asm("r10") = static_cast<i64>(casted_args[3]);
		register i64 r8 asm("r8") = static_cast<i64>(casted_args[4]);
		asm volatile("syscall"
					 : "=a"(ret)
					 : "a"(_syscall),
					   "D"(static_cast<i64>(casted_args[0])),
					   "S"(static_cast<i64>(casted_args[1])),
					   "d"(static_cast<i64>(casted_args[2])),
					   "r"(r10),
					   "r"(r8)
					 : "rcx", "r11", "memory");
	}

	else if constexpr (sizeof...(Args) == 6) {
		register i64 r10 asm("r10") = static_cast<i64>(casted_args[3]);
		register i64 r8 asm("r8") = static_cast<i64>(casted_args[4]);
		register i64 r9 asm("r9") = static_cast<i64>(casted_args[5]);
		asm volatile("syscall"
					 : "=a"(ret)
					 : "a"(_syscall),
					   "D"(static_cast<i64>(casted_args[0])),
					   "S"(static_cast<i64>(casted_args[1])),
					   "d"(static_cast<i64>(casted_args[2])),
					   "r"(r10),
					   "r"(r8),
					   "r"(r9)
					 : "rcx", "r11", "memory");
	}

	return ret;
}

i64 exit(i32 error) {
	i64 result{};
	asm("syscall" : "=a"(result) : "a"(SYS_EXIT), "D"(error));
	return check_for_error(result);
}

i64 read(Handle handle, char* buf, u64 count) {
	i64 result = syscall(SYS_READ, handle.handle, buf, count);
	return check_for_error(result);
}

i64 write(Handle handle, const char* buf, u64 count) {
	i64 result = syscall(SYS_WRITE, handle.handle, buf, count);
	return check_for_error(result);
}

i64 open(const char* pathname, i32 flags) {
	i64 result = syscall(SYS_OPEN, pathname, flags | FILE_CREAT, 0777);
	return check_for_error(result);
}

i64 close(Handle handle) {
	i64 result = syscall(SYS_CLOSE, handle.handle);
	return check_for_error(result);
}

void* allocate(u64 size) {
	i64 result = syscall(SYS_MMAP, nullptr, size + sizeof(u64), PROT_RDWR, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (check_for_error(result) == -1) {
		print_i64(get_last_error());
	}
	TK_ASSERT(check_for_error(result) != -1, "Memory allocation failed");
	return reinterpret_cast<void*>(reinterpret_cast<i64*>(result) + 1);
}

void deallocate(void* ptr) {
	u64 size = *reinterpret_cast<u64*>(ptr);
	i64 result = syscall(SYS_MUNMAP, ptr, size);
	TK_ASSERT(check_for_error(result) != -1, "Memory deallocation failed");
}

u64 time_nanoseconds() {
	struct timespec {
		long tv_sec;
		long tv_nsec;
	};
	timespec tspec{};

	i64 result = syscall(SYS_CLOCK_GETTIME, 0, &tspec);
	TK_ASSERT(check_for_error(result) != -1, "Error getting time");
	return static_cast<u64>(tspec.tv_sec) * 1000000000ULL + static_cast<u64>(tspec.tv_nsec);
}

}  // namespace pt

}  // namespace toki
