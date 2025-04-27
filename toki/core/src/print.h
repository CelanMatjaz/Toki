#pragma once

#include "string/string_view.h"

namespace toki {

void print(const StringView& view);

void println(const StringView& view);

void print_i64(i64 value, u32 radix = 10);

}  // namespace toki
