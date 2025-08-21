#pragma once

#include <toki/core/types.h>

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

template <typename T>
	requires(IsIntegralValue<T>)
u32 itoa(char* buf_out, T value, u32 radix = 10) {
	char buf[20]{};
	u32 offset = 0;
	bool add_minus = value < 0;

	if (value == 0) {
		buf_out[offset++] = '0';
		return offset;
	}

	while (value > 0) {
		buf[offset++] = value % radix + '0';
		value /= radix;
	}

	if (add_minus) {
		buf[offset++] = '-';
	}

	for (u32 i = 0; i < offset; i++) {
		buf_out[i] = buf[offset - i - 1];
	}

	return offset;
}

template <typename T>
	requires IsPointer<T>
u32 to_string(char* buf_out, T value) {
	constexpr char CHARS[] = "0123456789abcdef";
	char buf[20]{};
	u32 offset = 0;

	if (value == 0) {
		constexpr char NULLPTR[] = "nullptr";
		memcpy(NULLPTR, buf_out, sizeof(NULLPTR) - 1);
		return sizeof(NULLPTR) - 1;
	}

	u64 casted = reinterpret_cast<u64>(value);
	constexpr u64 RADIX = 16;
	while (casted > 0) {
		buf[offset++] = CHARS[casted % RADIX];
		casted /= RADIX;
	}

	buf_out[0] = '0';
	buf_out[1] = 'x';
	buf_out += 2;

	for (u32 i = 0; i < offset; i++) {
		buf_out[i] = buf[offset - i - 1];
	}

	return offset + 2;
}

}  // namespace toki
