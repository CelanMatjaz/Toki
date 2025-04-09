#include <sys/syscall.h>

#include "../../core/assert.h"
#include "../../core/string.h"
#include "../../utils/concepts.h"
#include "../print.h"
#include "linux_platform.h"
#include "types/type_traits.h"

namespace toki {

constexpr u32 PRINT_BUFFER_SIZE = 4096;

void print(const char* str) {
    print(1, str, toki::strlen(str));
}

void print(NativeHandle fd, const void* str, u64 n) {
    TK_ASSERT_PLATFORM_ERROR(toki::syscall(SYS_write, fd, str, n), "Could not write to fd");
}

void println(const char* str) {
    char buf[PRINT_BUFFER_SIZE]{};
    u64 length = toki::strlen(str);
    TK_ASSERT(length + 1 < sizeof(buf), "Buffer for printing not big enough, maybe time to write a loop here?");
    toki::memcpy(str, buf, length);
    buf[length] = '\n';
    print(1, buf, length + 1);
}

// template <typename T>
//     requires HasToStringFunctionConcept<T>
// constexpr void _printf(char* buffer, u32 buffer_offset, const char* fmt, T arg) {
//
// }
//
// template <typename T, typename... Args>
//     requires HasToStringFunctionConcept<T>
// constexpr void _printf(char* buffer, u32 buffer_offset, const char* fmt, T arg1, Args... args) {
//
// }

template <typename T>
constexpr void _arg_max_length() {
    return Type<T>::max_digits_10;
}

template <typename T, typename... Args>
constexpr u32 _arg_max_length() {
    return Type<T>::max_digits_10 + _arg_max_length<Args...>();
}

template <typename... Args>
constexpr void printf(const char* fmt, Args... args) {
    char buf[strlen(fmt) + _arg_max_length<Args...>()]{};
}

}  // namespace toki
