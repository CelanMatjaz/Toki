#pragma once

#include "../core/types.h"

namespace toki {

#if (defined(__GNUC__) || defined(__clang__)) && __cplusplus >= 202302L
#define TK_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define TK_UNREACHABLE() _STL_UNREACHABLE;
#else
#error "unreachable compiler function not found, only Clang, GCC and MSVC supported"
#endif

struct NativeHandle {
    NativeHandle(): handle(INVALID_HANDLE_ID) {}
#if defined(TK_PLATFORM_WINDOWS)
    constexpr static HANDLE INVALID_HANDLE_ID = ::INVALID_HANDLE_ID;
    NativeHandle(HANDLE handle): handle(handle) {}
    inline operator HANDLE() const {
        return handle;
    }
    HANDLE handle;
#elif defined(TK_PLATFORM_LINUX)
    constexpr static i64 INVALID_HANDLE_ID = -1;
    NativeHandle(i64 handle): handle(handle) {}
    inline operator i64() const {
        return handle;
    }
    i64 handle;
#endif
};

void debug_break();

const char* getenv(const char* var);

void exit(i32 error);

void* memory_allocate(u64 size);

void memory_free(void* ptr);

}  // namespace toki
