#pragma once

#include <toki/core/utils/path.h>
#include <toki/runtime/resources/resources.h>

namespace toki {

ResourceData load_texture(const Path& path);
void unload_texture(ResourceData& resource_data);

}  // namespace toki
