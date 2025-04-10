#pragma once

#include "../core/string.h"

namespace toki {

void println(StringView str);

template <typename... Args>
void println(StringView str, Args... args);

}  // namespace toki
