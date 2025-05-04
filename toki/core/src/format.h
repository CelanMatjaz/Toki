#pragma once

#include "core/assert.h"
#include "core/common.h"
#include "core/concepts.h"
#include "core/types.h"
#include "platform/attributes.h"
#include "string/string.h"

namespace toki {

template <typename Arg>
u32 _dump_single_arg(char* out, const Arg& arg) {
	if constexpr (IsIntegralValue<Arg>) {
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

}  // namespace toki
