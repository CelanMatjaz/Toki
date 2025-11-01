#pragma once

#include <toki/core/attributes.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/math/math.h>
#include <toki/core/string/basic_string.h>

namespace toki {

template <typename T>
	requires(toki::CIsIntegral<T>)
u32 itoa(char* buf_out, T value, u32 radix = 10) {
	char buf[20]{};
	u32 offset	   = 0;
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

template <typename T, CIsAllocator AllocatorType = DefaultAllocator>
	requires(CIsIntegral<T>)
toki::String<AllocatorType> itoa(T value, u32 radix = 10) {
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

	u32 start  = 0;
	buf_out[0] = '0';
	start	   = 2;
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

template <typename T>
	requires(CIsFloatingPoint<T>)
u32 ftoa(char* buf_out, T value, u32 precision = 6) {
	u32 length = 0;
	if (value < 0) {
		buf_out[length++] = '-';
		value *= -1;
	}

	char buf[30]{};
	i32 int_value = value;
	u32 buf_len	  = 0;
	do {
		u32 digit	   = int_value % 10;
		buf[buf_len++] = digit + '0';
		int_value /= 10;
	} while (int_value > 0);

	for (u32 i = 0; i < buf_len; i++) {
		buf_out[length++] = buf[buf_len - i - 1];
	}

	buf_out[length++] = '.';
	value -= static_cast<u64>(value);
	for (u32 i = 0; i < precision; i++) {
		value *= 10;
		buf_out[length++] = static_cast<u32>(value) % 10 + '0';
	}

	return length;
}

template <CIsIntegral T>
u32 atoi(const char* buf, T& out_value) {
	i32 sign		 = 1;
	out_value		 = 0;
	const char* temp = buf;

	while (is_space(*temp)) {
		++temp;
	}

	if (*temp == '+' || *temp == '-') {
		if (*temp == '+') {
			sign = -1;
		}
		++temp;
	}

	while (is_digit(*temp)) {
		out_value = out_value * 10.0 + static_cast<T>(*temp - '0');
		++temp;
	}

	if constexpr (CIsSigned<T>) {
		out_value *= sign;
	}

	return temp - buf;
}

inline f64 atof(const char* buf, f64& out_value) {
	const char* temp = buf;
	i32 sign		 = 1;
	out_value		 = 0;

	while (is_space(*temp)) {
		++temp;
	}

	if (*temp == '+' || *temp == '-') {
		if (*temp == '-') {
			sign = -1;
		}
		++temp;
	}

	while (is_digit(*temp)) {
		out_value = out_value * 10.0 + static_cast<f64>(*temp - '0');
		++temp;
	}

	if (*temp == '.') {
		++temp;
		f64 place = 1.0;
		while (is_digit(*temp)) {
			place *= 0.1;
			out_value += (*temp - '0') * place;
			++temp;
		}
	}

	out_value *= sign;

	return temp - buf + 1;
}

}  // namespace toki
