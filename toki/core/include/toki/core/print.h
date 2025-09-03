#pragma once

#include <toki/core/defines.h>
#include <toki/core/string/string_view.h>
#include <toki/core/utils/format.h>
#include <toki/platform/platform.h>
#include "toki/core/common.h"

namespace toki {

template <typename T, typename... Args>

void print(const toki::BasicStringView<T>& str, Args&&... args) {
	if constexpr (sizeof...(args) == 0) {
		toki::write(toki::STD_OUT, str.data(), str.size());
	} else {
		toki::String formatted = toki::format(str, toki::forward(args)...);
		toki::write(toki::STD_OUT, formatted.data(), formatted.size());
	}
}

template <typename T>
void print(const T* str) {
	print(BasicStringView{ str });
}

template <typename T, typename... Args>
void print(const T* str, Args&&... args) {
	print(BasicStringView{ str }, toki::forward(args)...);
}

template <typename T>
void print(const BasicString<T>& str) {
	print(BasicStringView{ str });
}

template <typename T, typename... Args>
void print(const BasicString<T>& str, Args&&... args) {
	print(BasicStringView{ str }, toki::forward(args)...);
}

}  // namespace toki
