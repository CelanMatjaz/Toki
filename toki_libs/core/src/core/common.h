#pragma once

#include "types.h"

namespace toki {

template <typename T>
inline u64 strlen(const T* str) {
    u64 len = 0;
    while (str[len++]) {}
    return len;
}

template <typename T>
inline b8 strcmp(const T* s1, const T* s2) {
    for (u32 i = 0;; i++) {
        if (s1[i] != s2[i]) {
            return false;
        } else if (s1[i] == 0 && s2[i] == 0) {
            return true;
        }
    }

    return false;
}

template <typename T>
inline void swap(T& t1, T& t2) {
    T temp = t1;
    *t1 = *t2;
    *t2 = temp;
}

inline void memcpy(void* src, void* dst, u32 size) {
    for (u32 i = 0; i < size; i++) {
        *reinterpret_cast<char*>(dst) = *reinterpret_cast<char*>(src);
    }
}

}  // namespace toki
