#pragma once

#include <toki/platform/platform.h>

namespace toki::resources {

enum class LoaderType {
	Text
};

template <LoaderType type, typename DataTypeIn, typename DataTypeOut>
class Loader {
public:
	static DataTypeIn read(const Path& path);
	static void write(const Path& path, const DataTypeOut& data);
};

}  // namespace toki::loaders
