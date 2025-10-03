#pragma once

#include <toki/core/attributes.h>
#include <toki/core/common/common.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/common/defines.h>
#include <toki/core/string/basic_string.h>
#include <toki/core/string/string_view.h>
#include <toki/core/utils/memory.h>
#include <toki/core/utils/string_dumpers.h>

namespace toki {

template <typename Arg>
	requires CHasDumpToString<Arg>
u32 _dump_single_arg(char* out, Arg&& arg) {
	using RawT = RemoveRef<typename RemoveConst<Arg>::type>::type;
	return StringDumper<RawT>::dump_to_string(out, toki::forward<Arg>(arg));
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
						offset += _dump_single_arg(&buf_out[offset], toki::forward<FirstArg>(arg));
						i += 2;
						start = i;

						if constexpr (sizeof...(args) > 0) {
							return offset + _format(
												static_cast<const char*>(&fmt[start]),
												&buf_out[offset],
												toki::forward<Args>(args)...);
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

	if constexpr (sizeof...(args) == 0) {
		if (u32 size = i - start; size > 0) {
			toki::memcpy(&fmt[start], &buf_out[offset], size);
			offset += size;
		}
	}

	return offset;
}

// TODO(Matjaž): add support for format properties in {}
template <typename... Args>
toki::String format(const toki::StringView& str, Args... args) {
	if constexpr (sizeof...(args) == 0) {
		return String{ str.data(), str.size() };
	}

	char buf_out[4096]{};
	u32 size = _format(str.data(), buf_out, toki::forward<Args>(args)...);
	return toki::String{ buf_out, size };
}

}  // namespace toki
