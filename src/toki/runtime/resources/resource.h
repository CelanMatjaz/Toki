#pragma once

#include <toki/core/types.h>
#include <toki/core/utils/path.h>
#include <toki/runtime/resources/resources.h>

namespace toki {

class Resource {
public:
	Resource() = default;
	Resource(const Path& path, ResourceType type);
	~Resource();

	DELETE_COPY(Resource)

	Resource(Resource&& other) {
		toki::swap(m_data, other.m_data);
	}

	Resource& operator=(Resource&& other) {
		toki::swap(m_data, other.m_data);
		return *this;
	}

	void load_as(const Path& path, ResourceType type);
	void unload();

	const ResourceMetadata& metadata() const {
		return m_data.resource_data.metadata;
	}

	u64 size() const {
		return m_data.resource_data.size;
	}

	const void* data() const {
		return m_data.resource_data.data;
	}

private:
	struct {
		ResourceData resource_data{};
		Path path{};
		ResourceType type{};
	} m_data;
};

}  // namespace toki
