#pragma once

#include "base.h"

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

}  // namespace toki
