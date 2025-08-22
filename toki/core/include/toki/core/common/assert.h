#pragma once

#include <toki/core/attributes.h>
#include <toki/core/common/macros.h>
#include <toki/core/utils/format.h>
#include <toki/core/utils/print.h>

namespace toki {

#define TK_ASSERT(condition, message, ...)                                                               \
	if (auto c = condition; !c) {                                                                        \
		toki::print("Assertion '" #condition "' failed in file " __FILE__ ":" AS_STRING(__LINE__) "\n"); \
		TK_DEBUG_BREAK();                                                                                \
	}  // namespace toki

}  // namespace toki
