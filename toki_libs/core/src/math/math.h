#pragma once

#include "math_types.h"

namespace toki {

constexpr f64 PI = 3.14159265358979;

template <typename T>
inline constexpr T min(T v1, T v2) {
    return v1 < v2 ? v1 : v2;
}

template <typename T>
inline constexpr T max(T v1, T v2) {
    return v1 > v2 ? v1 : v2;
}

template <typename T>
inline constexpr T clamp(T value, T min_value, T max_value) {
    return min(max(value, min_value), max_value);
}

template <typename T>
inline constexpr T pow(T value, u32 exp) {
    if (exp == 0) {
        return 1;
    }

    for (u32 i = 0; i < exp - 1; i++) {
        value *= value;
    }

    return value;
}

}  // namespace toki
