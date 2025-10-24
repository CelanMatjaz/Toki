#pragma once

#include <toki/core/core.h>
#include <toki/runtime/resources/resources.h>

namespace toki {

ResourceData load_text(const Path& path);
void unload_text(ResourceData& resource_data);

}  // namespace toki
