#pragma once

#include <toki/core/platform/syscalls.h>
#include <toki/core/string/string_view.h>
#include <toki/core/utils/format.h>

namespace toki {

template <typename... Args>
void print(const StringView str, Args&&... args) {
	if constexpr (sizeof...(args) == 0) {
		toki::write(toki::STD_OUT, str.data(), str.size());
	} else {
		toki::String formatted = toki::format(str.data(), toki::forward<Args>(args)...);
		toki::write(toki::STD_OUT, formatted.data(), formatted.size());
	}
}

template <typename... Args>
void println(const StringView str, Args&&... args) {
	print(str, toki::forward<Args>(args)...);
	print("\n");
}

}  // namespace toki
