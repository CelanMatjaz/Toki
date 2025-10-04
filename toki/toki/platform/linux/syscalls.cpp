#include <toki/platform/linux/syscalls.h>

#include "toki/core/common/type_traits.h"

namespace toki::platform {

// i64 syscall_raw(Syscall syscall, auto arg1, auto arg2, auto arg3, auto arg4, auto arg5, auto arg6) {
// 	long ret;
// 	register long r10 asm("r10") = arg4;
// 	register long r8 asm("r8") = arg5;
// 	register long r9 asm("r9") = arg6;
// 	asm volatile("syscall"
// 				 : "=a"(ret)
// 				 : "0"(static_cast<i64>(syscall)), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
// 				 : "rcx", "r11", "memory");
// }
//
// template <typename ReturnType, typename... Args>
// 	requires(sizeof...(Args) <= 6)
// toki::Expected<ReturnType, PlatformErrorDetailed> syscall_checked(Syscall syscall, Args... args) {
// 	i64 values[6] = { static_cast<i64>(args)... };
//
// 	auto result = syscall_raw(syscall, values[0], values[1], values[2], values[3], values[4], values[5]);
//
// 	if (result < 0) {
// 		return PlatformErrorDetailed{ PlatformError::Unknown, result };
// 	}
//
// 	if constexpr (CIsPointer<ReturnType>) {
// 		return reinterpret_cast<ReturnType>(result);
// 	} else {
// 		return static_cast<ReturnType>(result);
// 	}
// }

}  // namespace toki::platform
