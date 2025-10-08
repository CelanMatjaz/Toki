#pragma once

#include <toki/core/common/assert.h>
#include <toki/core/utils/format.h>
#include <toki/platform/syscalls.h>

namespace toki {

template <typename T, typename... Args>

void print(const toki::BasicStringView<T>& str, Args&&... args) {
	if constexpr (sizeof...(args) == 0) {
		platform::write(toki::STD_OUT, str.data(), str.size());
	} else {
		toki::String formatted = toki::format(str, toki::forward<Args>(args)...);
		platform::write(toki::STD_OUT, formatted.data(), formatted.size());
	}
}

template <typename T>
void print(const T* str) {
	print(BasicStringView{ str });
}

template <typename T, typename... Args>
void print(const T* str, Args&&... args) {
	print(BasicStringView{ str }, toki::forward<Args>(args)...);
}

template <typename T>
void print(const BasicString<T>& str) {
	print(BasicStringView{ str });
}

template <typename T, typename... Args>
void print(const BasicString<T>& str, Args&&... args) {
	print(BasicStringView{ str }, toki::forward<Args>(args)...);
}

template <typename T>
void println(const T* str) {
	print(BasicStringView{ str });
	print("\n");
}

template <typename T, typename... Args>
void println(const T* str, Args&&... args) {
	print(BasicStringView{ str }, toki::forward<Args>(args)...);
	print("\n");
}

template <typename T>
void println(const BasicString<T>& str) {
	print(BasicStringView{ str });
	print("\n");
}

template <typename T, typename... Args>
void println(const BasicString<T>& str, Args&&... args) {
	toki::String string(str, toki::strlen(str) + 1);
	string[string.size() - 1] = '\n';
	print(BasicStringView{ str }, toki::forward<Args>(args)...);
}

}  // namespace toki
