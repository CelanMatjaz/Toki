#pragma once

#include <vector>

#include "core/core.h"
#include "renderer/renderer_types.h"

namespace toki {

std::vector<u32> compile_shader(ShaderStage stage, std::string& source);

}  // namespace toki
