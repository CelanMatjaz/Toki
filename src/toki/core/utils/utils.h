#pragma once

#include <toki/core/utils/path.h>

#include "toki/core/utils/file.h"

namespace toki {

template <auto... Values>
constexpr b8 any_of(auto value) {
	return ((value == Values) || ...);
}

template <typename T>
	requires CIsArrayContainer<char, T>
void dump_to_file(Path path, const T& container) {
	File file(path, FileMode::WRITE, FileFlags::FILE_FLAG_CREATE);
	file.write(container.data(), container.size());
}

}  // namespace toki
