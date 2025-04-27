#pragma once

#include "platform/defines.h"
#include "types/types.h"

namespace toki {

#if (defined(__GNUC__) || defined(__clang__)) && __cplusplus >= 202302L
	#define TK_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
	#define TK_UNREACHABLE() _STL_UNREACHABLE;
#else
	#error "unreachable compiler function not found, only Clang, GCC and MSVC supported"
#endif

#if defined(__has_builtin)
	#if defined(_MSC_VER)
		#define TK_DEBUG_BREAK() __debugbreak()
	#elif defined(__clang__)
		#if __has_builtin(__builtin_debugtrap)
			#define TK_DEBUG_BREAK() __builtin_debugtrap();
		#else
			#define TK_DEBUG_BREAK() __builtin_trap();
		#endif
	#elif defined(__GNUC__)
		#define TK_DEBUG_BREAK() __builtin_trap();
	#endif
#endif

#if !defined(TK_DEBUG_BREAK)
	#error "TK_DEBUG_BREAK is required but not defined"
#endif

namespace pt {

constexpr u64 STD_IN = 0;
constexpr u64 STD_OUT = 1;
constexpr u64 STD_ERR = 2;

struct Handle {
	Handle(u64 value): handle(static_cast<i64>(value)) {}
	i64 handle;
};

extern u64 last_error;

inline u64 get_last_error() {
	return last_error;
}

i64 exit(i32 error);

i64 read(Handle handle, char* buf, u64 count);

i64 write(Handle handle, const char* buf, u64 count);

i64 open(const char* pathname, i32 flags);

i64 close(Handle handle);

void* allocate(u64 size);

void deallocate(void* ptr);

}  // namespace pt

}  // namespace toki
