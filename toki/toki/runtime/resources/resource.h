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

	void load_as(const Path& path, ResourceType type);
	void unload();

private:
	ResourceType m_type{};
	ResourceData m_data{};
	Path m_path{};
};

}  // namespace toki
