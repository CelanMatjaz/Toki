#pragma once

#include <toki/core/attributes.h>
#include <toki/core/common/macros.h>

namespace toki {

template <typename T>
void println(const T* str);

template <typename T, typename... Args>
void println(const T* str, Args&&... args);

#define TK_ASSERT(condition, ...)                                                             \
	if (auto c = static_cast<b8>(condition); !c) {                                                             \
		toki::println("Assertion '" #condition "' failed " __FILE__ ":" AS_STRING(__LINE__)); \
		__VA_OPT__(toki::println(__VA_ARGS__);)                                               \
		TK_DEBUG_BREAK();                                                                     \
	}  // namespace toki

}  // namespace toki
