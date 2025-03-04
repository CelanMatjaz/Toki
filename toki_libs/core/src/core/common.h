#pragma once

#include "base.h"

namespace toki {

template <typename T>
inline u64 strlen(const T* str) {
    u64 len = 0;
    while (str[len++]) {}
    return len;
}

}  // namespace toki
