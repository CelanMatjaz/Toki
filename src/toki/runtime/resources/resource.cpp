#include "toki/runtime/resources/resource.h"

#include <toki/runtime/allocators.h>
#include <toki/runtime/resources/loaders/text_loader.h>
#include <toki/runtime/resources/loaders/texture_loader.h>

namespace toki {

Resource::Resource(const Path& path, ResourceType type) {
	load_as(path, type);
}

Resource::~Resource() {
	if (m_data.resource_data.data != nullptr && m_data.resource_data.size > 0 && m_data.type != ResourceType::NONE) {
		unload();
	}
}

void Resource::load_as(const Path& path, ResourceType type) {
	switch (type) {
		case ResourceType::BINARY:
			break;
		case ResourceType::TEXT:
			m_data.resource_data = load_text(path);
			break;
		case toki::ResourceType::NONE:
			break;
		case ResourceType::TEXTURE:
			m_data.resource_data = load_texture(path);
			break;
	}

	m_data.type = type;
}

void Resource::unload() {
	switch (m_data.type) {
		case ResourceType::BINARY:
			break;
		case ResourceType::TEXT:
			unload_text(m_data.resource_data);
			break;
		case toki::ResourceType::NONE:
			break;
		case ResourceType::TEXTURE:
			unload_texture(m_data.resource_data);
			break;
	}

	m_data.type = ResourceType::NONE;
}

}  // namespace toki
