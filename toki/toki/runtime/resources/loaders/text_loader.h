#pragma once

#include <toki/core/core.h>
#include <toki/runtime/resources/loaders/loader.h>

namespace toki::resources {

template <>
toki::String Loader<LoaderType::Text, toki::String, toki::Span<byte>>::read(const Path& path);

}  // namespace toki::resources
