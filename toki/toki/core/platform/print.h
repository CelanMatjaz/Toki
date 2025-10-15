#pragma once

#include <toki/core/defines.h>
#include <toki/platform/platform.h>
#include <toki/core/string/string_view.h>
#include "toki/core/common.h"
#include "toki/core/string/basic_string.h"
#include "toki/core/utils/format.h"

namespace toki {

template <typename T, typename... Args>
void platform_print(BasicStringView<T> str, Args&&... args) {
	if constexpr (sizeof...(args) == 0) {
		toki::write(toki::STD_OUT, str.data(), str.size());
	} else {
		toki::String formatted = format(str, toki::forward(args)...);
		toki::write(toki::STD_OUT, formatted.data(), formatted.size());
	}
}

}  // namespace toki
