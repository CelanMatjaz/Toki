#include <toki/runtime/resources/resource.h>

#include "toki/runtime/resources/loaders/text_loader.h"

namespace toki {

Resource::Resource(const Path& path, ResourceType type) {
	load_as(path, type);
}

Resource::~Resource() {
	if (m_data.data != nullptr && m_data.size > 0 && m_type != ResourceType::NONE) {
		unload();
	}
}

void Resource::load_as(const Path& path, ResourceType type) {
	switch (type) {
		case ResourceType::BINARY:
		case ResourceType::TEXT:
			m_data = load_text<DefaultAllocator>(path);
			break;
		case toki::ResourceType::NONE:
			break;
	}

	m_type = type;
}

void Resource::unload() {
	switch (m_type) {
		case ResourceType::BINARY:
			break;
		case ResourceType::TEXT:
			m_data = unload_text<DefaultAllocator>(m_data);
			break;
		case toki::ResourceType::NONE:
			break;
	}

	m_type = ResourceType::NONE;
}

}  // namespace toki
