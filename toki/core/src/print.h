#pragma once

#include "core/common.h"
#include "format.h"
#include "platform/attributes.h"
#include "string/string_view.h"

namespace toki {

void print(const StringView& view);

void println(const StringView& view);

void print_i64(i64 value, u32 radix = 10);

template <typename... Args>
void print_args(Args&&... args) {
	constexpr u32 BUFFER_SIZE = 4096;
	char buf[BUFFER_SIZE]{};
	u32 byte_count = dump_args(buf, toki::move(args)...);
	if (byte_count >= BUFFER_SIZE) {
		print("Print buffer was writen to out of bounds");
		TK_UNREACHABLE();
	}
	buf[byte_count] = '\n';
	print(buf);
}

}  // namespace toki
