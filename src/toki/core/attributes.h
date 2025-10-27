#pragma once

namespace toki {

#if (defined(__GNUC__) || defined(__clang__))
	#define TK_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
	#define TK_UNREACHABLE() _STL_UNREACHABLE;
#else
	#error "builtin unreachable compiler function not found, only Clang, GCC and MSVC supported"
#endif

#if defined(__has_builtin)
	#if defined(_MSC_VER)
		#define TK_DEBUG_BREAK() __debugbreak()
	#elif defined(__clang__)
		#if __has_builtin(__builtin_trap)
			#define TK_DEBUG_BREAK() __builtin_trap();
		#else
			#define TK_DEBUG_BREAK() __builtin_debugtrap();
		#endif
	#elif defined(__GNUC__)
		#define TK_DEBUG_BREAK() __builtin_trap();
	#endif
#endif

#if !defined(TK_DEBUG_BREAK)
	#error "TK_DEBUG_BREAK is required but not defined"
#endif

}  // namespace toki
