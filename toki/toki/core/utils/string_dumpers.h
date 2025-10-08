#pragma once

#include <toki/core/common/common.h>
#include <toki/core/common/defines.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/string/converters.h>
#include <toki/core/utils/memory.h>

namespace toki {

template <typename T>
struct StringDumper {};

template <>
struct StringDumper<char*> {
	static constexpr u64 dump_to_string(char* out, const char* arg) {
		u64 length = toki::strlen(arg);
		toki::memcpy(out, arg, toki::strlen(arg));
		return length;
	}
};

static_assert(CHasDumpToString<char*>);

template <>
struct StringDumper<char> {
	static constexpr u64 dump_to_string(char* out, const char arg) {
		out[0] = arg;
		return 1;
	}
};

static_assert(CHasDumpToString<char>);

template <>
struct StringDumper<bool> {
	static constexpr u64 dump_to_string(char* out, const bool arg) {
		if (arg) {
			toki::memcpy(out, TRUE_STR, strlen(TRUE_STR));
			return strlen(TRUE_STR);
		} else {
			toki::memcpy(out, FALSE_STR, strlen(FALSE_STR));
			return strlen(FALSE_STR);
		}
	}
};

static_assert(CHasDumpToString<bool>);

template <typename T>
	requires CIsIntegral<T>
struct StringDumper<T> {
	static constexpr u64 dump_to_string(char* out, const T arg) {
		return itoa(out, remove_ref(arg));
	}
};

static_assert(CHasDumpToString<u8>);

// template <typename T, typename C>
// 	requires CHasToString<C, T>
// struct StringDumper<T> {
// 	static constexpr u32 dump_to_string(char* out, T&& arg) {
// 		return itoa(out, remove_ref(arg));
// 	}
// };
//
// template <typename T>
// 	requires CIsConvertible<T, i64>
// struct StringDumper<T> {
// 	static constexpr u32 dump_to_string(char* out, T&& arg) {
// 		return itoa(out, remove_r_value_ref(arg));
// 	}
// };

template <typename T>
	requires CIsPointer<T>
struct StringDumper<T> {
	static constexpr u32 dump_to_string(char* out, const T arg) {
		return itoa_pretty(out, arg, 16);
	}
};

static_assert(CHasDumpToString<u8>);

}  // namespace toki
