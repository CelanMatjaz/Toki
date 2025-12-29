#pragma once

#include <toki/core/types.h>

namespace toki {

using NativeHandle = i32;

// struct NativeHandle {
// #if defined(TK_PLATFORM_LINUX)
// 	using NativeHandleType								   = i32;
// 	static constexpr NativeHandleType INVALID_HANDLE_VALUE = -1;
// #endif
// 	NativeHandle(NativeHandleType value): handle(value) {}
// 	NativeHandleType handle;
//
// 	operator NativeHandleType() const {
// 		return handle;
// 	}
//
// 	operator NativeHandleType&()  {
// 		return handle;
// 	}
//
// 	b8 valid() const {
// 		return handle != INVALID_HANDLE_VALUE;
// 	}
//
// 	NativeHandle(): handle(INVALID_HANDLE_VALUE) {}
// };

}  // namespace toki
