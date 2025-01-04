#pragma once

#include <cstdint>
#include <vector>

#include "core/core.h"

namespace toki {

enum class ShaderStage {
    Vertex,
    Fragment
};

std::vector<u32> compile_shader(ShaderStage stage, std::string& source);

}  // namespace toki
