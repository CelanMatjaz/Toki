#include <unistd.h>

#include "../window/window.h"
#include "core/common.h"
#include "core/concepts.h"
#include "defines.h"
#include "memory/allocator.h"
#include "platform.h"
#include "platform/linux_platform.h"
#include "string/string.h"

extern char** environ;

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

const char* getenv(const char* var) {
	u32 length = strlen(var);
	char** env = environ;

	while (*env) {
		if (strcmp(var, *env, length) && (*env)[length] == '=') {
			return env[length + 1];
		}
		env++;
	}

	return nullptr;
}

inline static i64 check_for_error(i64 result) {
	if (result < 0) {
		last_error = -result;
		return -1;
	}
	return result;
}

template <typename T>
concept SyscallArgumentConcept = IsConvertibleConcept<T, i64>;

template <typename T>
constexpr T cast_to(auto&& value) {
	return (T) remove_r_value_ref(value);
}

template <typename T>
constexpr T cast_to(auto& value) {
	return (T) (value);
}

template <typename... Args>
	requires(SyscallArgumentConcept<Args> && ...) && (sizeof...(Args) <= 6)
static inline i64 syscall(u64 _syscall, Args... args) {
	i64 ret{};

	if constexpr (sizeof...(Args) == 0) {
		asm volatile("syscall" : "=a"(ret) : "a"(_syscall) : "rcx", "r11", "memory");
		return ret;
	}
	void* stack{};
	asm volatile("mov %%rsp, %0" : "=r"(stack));

	constexpr auto arg_count = sizeof...(Args);
	static_assert(arg_count <= 6);
	i64 casted_args[arg_count]{ (i64) (args)... };

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
	i64 result = syscall(SYS_READ, handle.handle, reinterpret_cast<i64>(buf), (i64) count);
	return check_for_error(result);
}

i64 write(Handle handle, const char* buf, u64 count) {
	i64 result = syscall(SYS_WRITE, handle.handle, reinterpret_cast<i64>(buf), count);
	return check_for_error(result);
}

Handle open(const char* pathname, i32 flags) {
	i64 result = syscall(SYS_OPEN, reinterpret_cast<i64>(pathname), flags | FILE_CREAT, 0777);
	return check_for_error(result);
}

i64 close(Handle handle) {
	i64 result = syscall(SYS_CLOSE, handle.handle);
	return check_for_error(result);
}

void* allocate(u64 size) {
	i64 result = syscall(SYS_MMAP, 0, size + sizeof(u64), PROT_RDWR, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	TK_ASSERT(check_for_error(result) != -1, "Memory allocation failed");
	*reinterpret_cast<i64*>(result) = size;
	return reinterpret_cast<i64*>(result) + 1;
}

void deallocate(void* ptr) {
	i64* size = reinterpret_cast<i64*>(ptr) - 1;
	i64 result = syscall(SYS_MUNMAP, size, static_cast<u64>(*size));
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

Handle socket_create() {
	i64 result = syscall(SYS_SOCKET, 1, 1, 0);
	TK_ASSERT(check_for_error(result) != -1, "Error creating socket");
	return result;
}

i64 socket_connect(Handle handle, void* addr, u64 addr_size) {
	i64 result = syscall(SYS_CONNECT, handle.handle, addr, addr_size);
	TK_ASSERT(check_for_error(result) != -1, "Error connecting to socket");
	return result;
}

Socket::Socket(): m_handle(socket_create()) {}

Socket::~Socket() {
	close(m_handle);
}

void Socket::connect(const StringView& path) {
	SocketAddr addr{};
	addr.sun_family = 1;
	toki::memcpy(path.data(), addr.sun_path, path.length());
}

void Socket::close() {
	::close(m_handle);
}

u32 Socket::send(const void* msg, u32 size) {
	return 0;
}

u32 Socket::recv(void* msg, u32 size) {
	return 0;
}

};

}  // namespace pt

}  // namespace toki
