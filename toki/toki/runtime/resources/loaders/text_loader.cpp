#include <toki/runtime/resources/loaders/text_loader.h>

namespace toki::resources {

template <>
toki::String Loader<LoaderType::Text, toki::String, toki::Span<byte>>::read([[maybe_unused]] const Path& path) {
	return {};
}

}  // namespace toki::resources
