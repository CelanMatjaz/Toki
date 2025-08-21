#pragma once

#include <toki/core/platform/print.h>
#include <toki/core/string/basic_string.h>
#include <toki/core/string/string_view.h>

namespace toki {

template <typename T>
void print(const T* str) {
	platform_print(BasicStringView{ str });
}

template <typename T, typename... Args>
void print(const T* str, Args&&... args) {
	platform_print(BasicStringView{ str }, toki::forward(args)...);
}

template <typename T>
void print(const BasicString<T>& str) {
	platform_print(BasicStringView{ str });
}

template <typename T, typename... Args>
void print(const BasicString<T>& str, Args&&... args) {
	platform_print(BasicStringView{ str }, toki::forward(args)...);
}

template <typename T>
void print(const BasicStringView<T>& str) {
	platform_print(str);
}

template <typename T, typename... Args>
void print(const BasicStringView<T>& str, Args&&... args) {
	platform_print(BasicStringView{ str }, toki::forward(args)...);
}

}  // namespace toki
