#pragma once

#include <toki/core/defines.h>
#include <toki/core/platform/platform.h>
#include <toki/core/string/string_view.h>

namespace toki {

template <typename T, typename... Args>

void platform_print(BasicStringView<T> str, Args&&... args) {
	if constexpr (sizeof...(args) == 0) {
	} else {
		toki::write(toki::STD_OUT, str.data(), str.size());
	}
}

}  // namespace toki
