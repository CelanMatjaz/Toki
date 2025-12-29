#pragma once

#include <toki/core/types.h>

namespace toki {

constexpr const u64 MAX_STACK_TRACE_FRAME_COUNT = 64;

struct StackFrame {
	void* address;
};

struct StackTrace {
	StackFrame frames[MAX_STACK_TRACE_FRAME_COUNT]{};
	u64 stack_frame_count{};
};

inline StackTrace capture_stack_trace(StackFrame* frames_out, [[maybe_unused]] u64 max_count) {
	StackTrace stack_trace{};

	void* addr			= __builtin_return_address(0);
	frames_out->address = addr;

	// for (u32 level = 1; level < max_count + 1; level++) {
	// }

	return stack_trace;
}

}  // namespace toki
