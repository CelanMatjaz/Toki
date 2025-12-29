#pragma once

#include <toki/core/platform/syscalls.h>
#include <toki/core/string/string_view.h>
#include <toki/core/utils/format.h>

namespace toki {

template <typename... Args>
void print(StringView str, Args&&... args) {
	if constexpr (sizeof...(args) == 0) {
		TokiError _ = toki::write(toki::STD_OUT, str.data(), str.size());
	} else {
		toki::String formatted = toki::format<Args...>(str.data(), toki::forward<Args>(args)...);
		TokiError _			   = toki::write(toki::STD_OUT, formatted.data(), formatted.size());
	}
}

template <typename... Args>
void println(StringView str, Args&&... args) {
	char* string = reinterpret_cast<char*>(DefaultAllocator::allocate(str.size() + 2));
	toki::memcpy(string, str.data(), str.size());
	string[str.size()]	   = '\n';
	string[str.size() + 1] = '\0';
	print(string, toki::forward<Args>(args)...);
}

}  // namespace toki
