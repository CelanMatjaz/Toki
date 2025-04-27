#include <sys/syscall.h>

#include "../../core/assert.h"
#include "../../core/string.h"
#include "../../utils/concepts.h"
#include "../print.h"
#include "linux_platform.h"
#include "math/math.h"
#include "types/type_traits.h"

namespace toki {

constexpr u32 PRINT_BUFFER_SIZE = 4096;

static void _print(NativeHandle fd, const void* str, u64 n) {
    TK_ASSERT_PLATFORM_ERROR(toki::syscall(SYS_write, fd, str, n), "Could not write to fd");
}

// void println(const char* str) {
//     char buf[PRINT_BUFFER_SIZE]{};
//     u64 length = toki::strlen(str);
//     TK_ASSERT(length + 1 < sizeof(buf), "Buffer for printing not big enough, maybe time to write a loop here?");
//     toki::memcpy(str, buf, length);
//     buf[length] = '\n';
//     print(1, buf, length + 1);
// }

void println(StringView str) {
    TK_ASSERT(str.length() < PRINT_BUFFER_SIZE - 1, "Long prints are currently not supported");
    char buf[PRINT_BUFFER_SIZE];
    toki::memcpy(str.data(), buf, str.length());
    _print(1, buf, str.length() + 1);
    buf[str.length()] = '\n';
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
//

template <u32 SIZE = PRINT_BUFFER_SIZE>
struct PrintBuffer {
    char data[SIZE]{};
    u64 offset = 0;

    void flush(NativeHandle fd) {
        _print(1, data, offset);
    }
};

template <typename T>
    requires IsIntegralValue<T>
u32 to_string(const T value, char* buf_out) {
    if (value == 0) {
        buf_out[0] = '0';
        return 1;
    }

    char buf[20]{};
    u32 offset = 0;

    T temp = abs(value);
    while (temp != 0) {
        auto a = temp % 10;
        buf[offset++] = '0' + temp % 10;
        temp /= 10;
    }

    if (value < 0) {
        buf[offset++] = '-';
    }

    for (u32 i = 0; i < offset; i++) {
        buf_out[i] = buf[offset - i - 1];
    }

    return offset;
}

template <typename T, typename... Args>
    requires HasToStringFunctionConcept<T>
static void _print_formatted(PrintBuffer<PRINT_BUFFER_SIZE>& buffer, T&& arg, Args&&... args) {
    toki::to_string(arg, buffer.data);

    if constexpr (sizeof...(args) == 0) {
        return;
    } else {
        _print_formatted(buffer, args...);
    }
}

template <typename... Args>
void println(StringView str, Args&&... args) {
    PrintBuffer<PRINT_BUFFER_SIZE> buffer;
}

}  // namespace toki
