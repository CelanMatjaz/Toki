#pragma once

#include <cstdio>

#include "types.h"

namespace toki {

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

template <typename T, typename... Args>
inline void emplace(void* dst, Args&&... args) {
    new (dst) T(args...);
}

template <typename T>
inline constexpr u64 strlen(const T* str) {
    u64 len = 0;
    while (str[++len]) {}
    return len;
}

template <typename T>
inline constexpr b8 strcmp(const T* s1, const T* s2, u32 length = 0) {
    for (u32 i = 0; i < length || !length; i++) {
        if (s1[i] != s2[i]) {
            return false;
        }
        if (s1[i] == 0 && s2[i] == 0) {
            return true;
        }
    }

    return true;
}

template <typename T>
inline T& remove_r_value_ref(T&& value) {
    return static_cast<T&>(value);
}

template <typename T>
inline void swap(T& t1, T& t2) {
    T& temp = t1;
    t1 = t2;
    t2 = temp;
}

template <typename T>
inline T&& move(const T& value) {
    return static_cast<T&&>(const_cast<T&>(value));
}

inline void memcpy(const void* src, void* dst, u32 size) {
    for (u32 i = 0; i < size; i++) {
        *&reinterpret_cast<char*>(dst)[i] = *&reinterpret_cast<const char*>(src)[i];
    }
}

}  // namespace toki
