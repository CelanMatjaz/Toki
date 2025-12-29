#pragma once

#include <toki/core/common/common.h>
#include <toki/core/common/defines.h>
#include <toki/core/common/type_traits.h>
#include <toki/core/string/converters.h>
#include <toki/core/utils/memory.h>

namespace toki {

template <typename T>
struct StringDumper {};

template <u64 N>
struct StringDumper<char[N]> {
	static constexpr u64 dump_to_string(char* out, const char arg[N]) {
		u64 length = toki::strlen(arg);
		toki::memcpy(out, arg, toki::strlen(arg));
		return length;
	}
};

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

static_assert(CHasDumpToString<b8>);

template <CIsIntegral T>
struct StringDumper<T> {
	static constexpr u64 dump_to_string(char* out, const typename RemoveConstVolatile<T>::type arg) {
		return itoa(out, remove_ref(arg));
	}
};

#define X(type) static_assert(CHasDumpToString<type>);
TOKI_INTEGER_TYPES_X
#undef X

template <CIsFloatingPoint T>
struct StringDumper<T> {
	static constexpr u64 dump_to_string(char* out, const T arg) {
		return ftoa(out, arg, 6);
	}
};

static_assert(CHasDumpToString<f32>);
static_assert(CHasDumpToString<f64>);
static_assert(CHasDumpToString<f128>);

template <CIsPointer T>
struct StringDumper<T> {
	static constexpr u64 dump_to_string(char* out, const T arg) {
		return itoa_pretty(out, arg, 16);
	}
};

static_assert(CHasDumpToString<void*>);

}  // namespace toki
