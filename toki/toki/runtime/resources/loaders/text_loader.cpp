#include <toki/runtime/resources/loaders/text_loader.h>

namespace toki::resources {

template <>
toki::String Loader<LoaderType::Text, toki::String, toki::Span<byte>>::read(const Path& path) {
	return {};
}

}  // namespace toki::resources
