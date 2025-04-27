#pragma once

#include "../core/types.h"

namespace toki {

template <typename T>
inline constexpr u32 strlen(const T* str) {
	u64 len = 0;
	while (str[++len]) {}
	return len;
}

inline constexpr b8 strcmp(const char* s1, const char* s2, u32 length = 0) {
	for (; *s1 && (*(s1++) == *(s2++)) && (length >= 0); --length) {}
	return (*s1 == 0 && *s2 == 0);
}

inline void memcpy(const void* src, void* dst, u32 size) {
	for (u32 i = 0; i < size; i++) {
		*&reinterpret_cast<char*>(dst)[i] = *&reinterpret_cast<const char*>(src)[i];
	}
}

}  // namespace toki
