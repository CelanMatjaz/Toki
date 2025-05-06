#pragma once

#include "../platform/attributes.h"
#include "../string/string_view.h"
#include "macros.h"

namespace toki {

void print(const StringView& view);

template <typename... Args>
void print_args(Args&&... args);

#define TK_ASSERT(condition, message, ...)                                                             \
	if (auto c = condition; !c) {                                                                      \
		toki::print("Assertion " #condition " failed in file " __FILE__ ":" AS_STRING(__LINE__) "\n"); \
		toki::print_args(message __VA_OPT__(, ) __VA_ARGS__);                                          \
		TK_DEBUG_BREAK();                                                                              \
	}  // namespace toki

#define TK_ASSERT_OR_RETURN(condition, return_value) \
	if (auto c = condition; !c) {                    \
		return return_value;                         \
	}

}  // namespace toki
