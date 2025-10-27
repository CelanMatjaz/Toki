#pragma once

#include <toki/core/attributes.h>
#include <toki/core/common/macros.h>
#include <toki/core/string/string_view.h>

namespace toki {

template <typename... Args>
void println(const auto str, Args&&... args);

#define TK_ASSERT(condition, ...)                                                                               \
	if (auto c = static_cast<b8>(condition); !c) {                                                              \
		toki::println("\n\033[31mAssertion '" #condition "' failed " __FILE__ ":" AS_STRING(__LINE__) "\033[0m"); \
		__VA_OPT__(toki::println(__VA_ARGS__);)                                                                 \
		TK_DEBUG_BREAK();                                                                                       \
	}  // namespace toki

}  // namespace toki
