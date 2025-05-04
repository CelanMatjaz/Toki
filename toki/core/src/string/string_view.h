#pragma once

#include "span.h"
#include "string.h"

namespace toki {

class StringView : public Span<char> {
public:
	StringView(const char* str): Span<char>(str, toki::strlen(str)) {}
	StringView(const char* str, u32 length): Span<char>(str, length) {}
};

}  // namespace toki
