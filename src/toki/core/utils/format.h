#pragma once

#include <toki/core/attributes.h>
#include <toki/core/common/assert.h>
#include <toki/core/common/common.h>
#include <toki/core/common/defines.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/string/basic_string.h>
#include <toki/core/utils/memory.h>
#include <toki/core/utils/string_dumpers.h>
#include <toki/core/utils/string_formatters.h>

namespace toki {

template <typename FirstArg, typename... Args>
u32 format_to(const char* fmt, char* buf_out, const FirstArg& arg, Args&&... args);

template <CIsAllocator AllocatorType>
struct Formatter<StringView, AllocatorType> {
	inline static constexpr const char* format_string = "{}";

	static constexpr toki::String<AllocatorType> format(const StringView& str) {
		return format(format_string, str);
	}

	static constexpr u64 format_to(char* buf_out, const StringView& str) {
		return ::toki::format_to(format_string, buf_out, str.data());
	}
};

static_assert(CHasStringFormatter<StringView>);

template <typename Arg>
	requires(CHasDumpToString<typename RemoveConst<Arg>::type>)
u32 dump_single_arg(char* out, const Arg& arg) {
	using RawT = typename RemoveConst<Arg>::type;
	return StringDumper<RawT>::dump_to_string(out, arg);
}

template <u32 N>
inline u32 read_buffer_until_character(const char* str, char* buf_out, char c) {
	for (u32 i = 0; i < N; i++) {
		if (str[i] == c) {
			toki::memcpy(buf_out, str, i + 1);
			return i + 1;
		}
	}

	return 0;
}

template <typename FirstArg, typename... Args>
u32 format_to(const char* fmt, char* buf_out, const FirstArg& arg, Args&&... args) {
	u32 offset		   = 0;
	u32 fmt_copy_start = 0;
	u32 i			   = 0;

	auto copy_bytes = [&](u32 copy_len) {
		toki::memcpy(&buf_out[offset], &fmt[fmt_copy_start], copy_len + 1);
		offset += copy_len + 1;
		fmt_copy_start = i + 2;
		i++;
	};

	for (; fmt[i] != 0; i++) {
		u32 copy_len = i - fmt_copy_start;
		switch (fmt[i]) {
			case '}': {
				TK_ASSERT(fmt[i + 1] != 0);
				if (fmt[i + 1] == '}') {
					copy_bytes(copy_len);
					break;
				}
				TK_UNREACHABLE();
				break;
			}
			case '{': {
				TK_ASSERT(fmt[i + 1] != 0);
				if (fmt[i + 1] == '{') {
					copy_bytes(copy_len);
					break;
				}

				char buf[64]{};
				u32 len = read_buffer_until_character<ARRAY_SIZE(buf)>(&fmt[i], buf, '}');
				TK_ASSERT(len == 2);

				// Copy everything to the left of '{}'
				toki::memcpy(&buf_out[offset], &fmt[fmt_copy_start], copy_len);
				offset += copy_len;

				fmt_copy_start = i + 1;

				if constexpr (CHasStringFormatter<FirstArg>) {
					offset += Formatter<FirstArg>::format_to(&buf_out[offset], arg);
				} else if constexpr (CHasDumpToString<typename RemoveConst<FirstArg>::type>) {
					offset += dump_single_arg(&buf_out[offset], arg);
				} else {
					static_assert(TypeFalseType<FirstArg>::value);
				}

				if constexpr (sizeof...(Args) > 0) {
					offset += format_to(&fmt[fmt_copy_start + 1], &buf_out[offset], toki::forward<const Args>(args)...);
					return offset;
				} else {
					i++;
					fmt_copy_start = i + 1;
				}
			}
		}
	}

	u32 copy_len = i - fmt_copy_start;
	if (copy_len > 0) {
		toki::memcpy(&buf_out[offset], &fmt[fmt_copy_start], copy_len);
		offset += copy_len;
	}

	return offset;
}

// TODO(Matjaž): add support for format properties in {}
template <typename... Args>
toki::String<DefaultAllocator> format(const char* str, Args... args) {
	if constexpr (sizeof...(args) == 0) {
		return String{ str };
	}

	char buf_out[4096]{};
	u32 size = format_to(str, buf_out, toki::forward<Args>(args)...);
	TK_ASSERT(size <= 4096);
	return toki::String{ buf_out, size };
}

}  // namespace toki
