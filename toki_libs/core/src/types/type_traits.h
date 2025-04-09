#pragma once

#include "../utils/concepts.h"
#include "math/math.h"
#include "types.h"

namespace toki {

template <typename T, u32 Radix = 10>
    requires IsIntegralValue<T>
consteval u32 max_print_digit_count();

template <typename T>
struct Type {
    static constexpr u64 size = sizeof(T);
    static constexpr T min = IsSignedValue<T> ? static_cast<T>(static_cast<T>(1) << (sizeof(T) * 8 - 1)) : 0;
    static constexpr T max = IsSignedValue<T> ? ~min : ~static_cast<T>(0);
    static constexpr u32 max_digits_10 = max_print_digit_count<T, 10>();
    static constexpr u32 max_digits_2 = max_print_digit_count<T, 2>();
    static constexpr u32 max_digits_16 = max_print_digit_count<T, 16>();
};

template <typename T, u32 Radix>
    requires IsIntegralValue<T>
consteval u32 max_print_digit_count() {
    if constexpr (Radix == 16) {
        return sizeof(T) / 8 + 2;
    }

    else if constexpr (Radix == 2) {
        return sizeof(T) * 8 + 2;
    }

    else if constexpr (Radix == 10) {
        T max = Type<T>::max;
        T min = Type<T>::min;

        u32 max_len = 0;
        while (max > 0) {
            max /= 10;
            max_len++;
        }

        return max_len + IsSignedValue<T>; // Not valid for 8 byte wide types, but good enough
    }
}

constexpr u16 a = ~0;

void asd() {
    constexpr u64 max_digits = Type<u16>::max_digits_10;
    constexpr u64 max_digits2 = Type<u16>::max_digits_2;
    constexpr u64 max_digits16 = Type<u16>::max_digits_16;
}

}  // namespace toki
