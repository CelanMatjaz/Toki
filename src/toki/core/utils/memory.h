#pragma once

#include <toki/core/types.h>

namespace toki {

template <typename T>
constexpr u64 strlen(const T* str) {
	u64 len = 0;
	while (str[len]) {
		++len;
	}
	return len;
}

constexpr b8 strcmp(const char* s1, const char* s2) {
	while (*s1 && (*(s1++) == *(s2++))) {}
	return (*s1 == 0 && *s2 == 0);
}

constexpr int strncmp(const char* s1, const char* s2, u32 length) {
	for (; length > 0; --length, ++s1, ++s2) {
		unsigned char c1 = static_cast<unsigned char>(*s1);
		unsigned char c2 = static_cast<unsigned char>(*s2);
		if (c1 != c2) {
			return c1 - c2;
		}
		if (c1 == 0) {
			return 0;
		}
	}
	return 0;
}

constexpr void memcpy(void* dst, const void* src, u64 size) {
	for (u32 i = 0; i < size; i++) {
		*&reinterpret_cast<byte*>(dst)[i] = *&reinterpret_cast<const byte*>(src)[i];
	}
}

constexpr void memset(void* dst, u32 size, toki::byte ch) {
	for (u32 i = 0; i < size; i++) {
		reinterpret_cast<toki::byte*>(dst)[i] = ch;
	}
}

constexpr b8 is_space(char c) {
	return c == ' ';
}

constexpr b8 is_digit(char c) {
	return c >= '0' && c <= '9';
}

constexpr u32 find_index_of(const char* buf, u32 length, char c) {
	for (u32 i = 0; i < length; i++) {
		if (buf[i] == c) {
			return i;
		}
	}

	return length;
}

constexpr b8 starts_with(const char* string, const char* substring) {
	return strncmp(string, substring, toki::strlen(substring)) == 0;
}

}  // namespace toki
