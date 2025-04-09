#pragma once

#include "platform.h"

namespace toki {

void print(const char* str);

void print(NativeHandle fd, const char* str, u64 n);

void println(const char* str);

}  // namespace toki
