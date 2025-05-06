#pragma once

#include "containers/basic_ref.h"
#include "containers/weak_ref.h"
#include "core/common.h"
#include "core/concepts.h"
#include "core/types.h"
#include "platform/attributes.h"
#include "string/string.h"

namespace toki {

template <typename Arg>
u32 _dump_single_arg(char* out, const Arg& arg) {
	if constexpr (IsSameValue<Arg, char>) {
		out[0] = arg;
		return 1;
	} else if constexpr (IsIntegralValue<Arg>) {
		return itoa(out, remove_r_value_ref(arg));
	} else if constexpr (IsSameValue<Arg, const char*> || IsSameValue<Arg, const char&> || IsCArray<Arg>) {
		u64 length = toki::strlen(arg);
		toki::memcpy(arg, out, toki::strlen(arg));
		return length;
	} else if constexpr (ToStringFunctionExistsConcept<Arg>) {
		return to_string(out, arg);
	} else if constexpr (IsPointer<Arg>) {
		return to_string(out, arg);
	} else if constexpr (IsConvertibleConcept<Arg, i64>) {
		return itoa(out, remove_r_value_ref((i64) (arg)));
	} else {
		static_assert(false, "Unhandled or invalid argument type provided to format");
		TK_UNREACHABLE();
	}
}

template <typename FirstArg, typename... Args>
u32 _dump_args(char* out, const FirstArg& arg, const Args&... args) {
	u64 bytes = _dump_single_arg(out, toki::move(arg));

	if constexpr (sizeof...(Args) > 0) {
		out[bytes++] = ' ';
		bytes += _dump_args(out + bytes, toki::move(args)...);
	}

	return bytes;
}

// TODO(Matjaž): Add buffer bound checking
template <typename... Args>
u32 dump_args(char* out, Args&&... args) {
	return _dump_args(out, toki::move(args)...);
}

template <typename FirstArg>
u32 _format_string(char* buf_out, const char* fmt, FirstArg&& arg) {
	for (u32 i = 0; fmt[i] != 0; i++) {
		if (fmt[i] == '{') {
			switch (fmt[i + 1]) {
				case '\0': {
					return 0;
				} break;
				case '}':
					toki::memcpy(fmt, buf_out, i);
					u32 buf_out_offset = i + _dump_single_arg(buf_out + i, arg);
					return buf_out_offset;
			}
		}
	}

	return 0;
}

template <typename FirstArg, typename... Args>
u32 _format_string(char* buf_out, const char* fmt, FirstArg&& arg, Args&&... args) {
	u32 length = 0;
	for (u32 i = 0; fmt[i] != 0; i++) {
		if (fmt[i] == '{') {
			switch (fmt[i + 1]) {
				case '\0': {
					return 0;
				} break;
				case '}':
					toki::memcpy(fmt, buf_out, i);
					u32 buf_out_offset = i + _dump_single_arg(buf_out + i, arg);

					if constexpr (sizeof...(args) == 0) {
						return buf_out_offset;
					} else {
						u32 byte_count = _format_string(buf_out + buf_out_offset, fmt + i + 2, toki::move(args)...);
						return buf_out_offset + byte_count;
					}
			}
		}

		length++;
	}
	toki::memcpy(fmt, buf_out, toki::strlen(fmt));
	return length;
}

template <typename... Args>
auto format_string(const char* fmt, const Args... args) {
	if constexpr (sizeof...(args) > 0) {
		char buffer[4096]{};
		u32 byte_count = _format_string(buffer, fmt, toki::move(args)...);
		return BasicRef<char>(byte_count + 1, buffer);
	} else {
		auto d = BasicRef<char>(toki::strlen(fmt), fmt);
		return d;
	}
}

}  // namespace toki
