#pragma once

#include <toki/core/attributes.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/string/basic_string.h>

namespace toki {

template <typename T>
	requires(toki::CIsIntegral<T>)
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
	requires(CIsIntegral<T>)
toki::String itoa(T value, u32 radix = 10) {
	char buffer[32]{};
	u32 length = itoa(buffer, value, radix);
	return toki::String{ buffer, length };
}

template <typename T>
	requires(CIsPointer<T> || CIsIntegral<T>)
u32 itoa_pretty(char* buf_out, T value, u32 radix = 10) {
	constexpr char CHARS[] = "0123456789abcdef";
	char buf[64]{};
	u32 offset = 0;

	if constexpr (CIsPointer<T>) {
		if (value == 0) {
			constexpr const char NULLPTR[] = "nullptr";
			memcpy(buf_out, NULLPTR, sizeof(NULLPTR) - 1);
			return sizeof(NULLPTR) - 1;
		}
	}

	u64 casted = reinterpret_cast<u64>(value);
	while (casted > 0) {
		buf[offset++] = CHARS[casted % radix];
		casted /= radix;
	}

	u32 start = 0;
	buf_out[0] = '0';
	start = 2;
	switch (radix) {
		case 2: {
			buf_out[1] = 'b';
		} break;
		case 8: {
			buf_out[1] = 'o';
		} break;
		case 16: {
			buf_out[1] = 'x';
		} break;
		default:
			TK_UNREACHABLE();
	}

	for (u32 i = 0; i < offset; i++) {
		buf_out[start + i] = buf[offset - i - 1];
	}

	return offset + start;
}

}  // namespace toki
