#pragma once

#include "core/macros.h"
#include "platform/platform.h"
#include "print.h"

namespace toki {

#define TK_ASSERT(condition, message)                                                                  \
	if (auto c = condition; !c) {                                                                      \
		toki::print("Assertion " #condition " failed in file " __FILE__ ":" AS_STRING(__LINE__) "\n"); \
		TK_DEBUG_BREAK();                                                                              \
	}

}  // namespace toki
