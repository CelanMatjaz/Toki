#pragma once

#include <toki/core/attributes.h>
#include <toki/core/common/concepts.h>
#include <toki/core/defines.h>
#include <toki/core/string/basic_string.h>
#include <toki/core/string/string_view.h>
#include <toki/core/utils/memory.h>

#include "toki/core/common.h"

namespace toki {

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
	requires(IsIntegralValue<T>)
toki::String itoa(T value, u32 radix = 10) {
	char buffer[32]{};
	u32 length = itoa(buffer, value, radix);
	return toki::String{ buffer, length };
}

template <typename T>
	requires(IsPointer<T> || IsIntegralValue<T>)
u32 to_string(char* buf_out, T value, u32 radix = 10) {
	constexpr char CHARS[] = "0123456789abcdef";
	char buf[64]{};
	u32 offset = 0;

	if constexpr (IsPointer<T>) {
		if (value == 0) {
			constexpr char NULLPTR[] = "nullptr";
			memcpy(NULLPTR, buf_out, sizeof(NULLPTR) - 1);
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

template <typename T>
	requires(IsPointer<T> || IsIntegralValue<T>)
toki::String to_string(T value, u32 radix = 10) {
	char buffer[64]{};
	u32 length = to_string(buffer, value, radix);
	return toki::String{ buffer, length };
}

template <typename Arg>
u32 _dump_single_arg(char* out, const Arg& arg) {
	if constexpr (IsSameValue<Arg, char>) {
		out[0] = arg;
		return 1;
	} else if constexpr (IsSameValue<Arg, bool>) {
		if (arg) {
			toki::memcpy(TRUE_STR, out, strlen(TRUE_STR));
			return strlen(TRUE_STR);
		} else {
			toki::memcpy(FALSE_STR, out, strlen(FALSE_STR));
			return strlen(FALSE_STR);
		}
	} else if constexpr (IsIntegralValue<Arg>) {
		return itoa(out, remove_r_value_ref(arg));
	} else if constexpr (IsSameValue<Arg, const char*> || IsSameValue<Arg, const char&> || IsCArray<Arg>) {
		u64 length = toki::strlen(arg);
		toki::memcpy(arg, out, toki::strlen(arg));
		return length;
	} else if constexpr (ToStringFunctionExistsConcept<Arg>) {
		return to_string(out, arg);
	} else if constexpr (IsPointer<Arg>) {
		return to_string(out, arg, 16);
	} else if constexpr (IsConvertibleConcept<Arg, i64>) {
		return itoa(out, remove_r_value_ref((i64) (arg)));
	} else {
		static_assert(false, "Unhandled or invalid argument type provided to format");
		TK_UNREACHABLE();
	}
}

template <typename FirstArg, typename... Args>
u32 _format(const char* fmt, char* buf_out, FirstArg&& arg, Args&&... args) {
	u32 offset = 0;
	u32 start = 0;
	u32 i = 0;

	for (; fmt[i]; i++) {
		switch (fmt[i]) {
			case '{': {
				if ((i - start) > 0) {
					toki::memcpy(&fmt[start], &buf_out[offset], i - start);
					offset += i - start;
				}

				start = i;
				switch (fmt[i + 1]) {
					case '\0': {
						TK_UNREACHABLE();
					} break;
					case '{': {
						buf_out[offset++] = '{';
						i++;
						start += 2;
					} break;
					case '}': {
						offset += _dump_single_arg(&buf_out[offset], arg);
						i += 2;
						start = i;

						if constexpr (sizeof...(args) > 0) {
							return offset +
								   _format(
									   static_cast<const char*>(&fmt[start]), &buf_out[offset], toki::forward(args)...);
						}
						continue;
					} break;
				}
			} break;
			case '}': {
				switch (fmt[i + 1]) {
					case '}': {
						buf_out[offset++] = '}';
						i++;
						start += 2;
					} break;
					default:
						TK_UNREACHABLE();
				}
			} break;
			default:
				break;
		}
	}

	// if constexpr (sizeof...(args) == 0) {
	// 	if (u32 size = i - start; size > 0) {
	// 		toki::memcpy(&fmt[start], &buf_out[offset], size);
	// 		offset += size;
	// 	}
	// }

	return offset;
}

// TODO(Matjaž): add support for format properties in {}
template <typename... Args>
toki::String format(const toki::StringView& str, Args... args) {
	if constexpr (sizeof...(args) == 0) {
		return String{ str.data(), str.size() };
	}

	char buf_out[4096]{};
	u32 size = _format(str.data(), buf_out, toki::forward(args)...);
	return toki::String{ buf_out, size };
}

}  // namespace toki
