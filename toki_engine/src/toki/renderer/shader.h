#pragma once

#include <filesystem>

#include "core/macros.h"

namespace toki {

class Shader {
public:
    struct Config {
        std::filesystem::path vertex_shader_path{};
        std::filesystem::path fragment_shader_path{};
    };
};

}  // namespace toki
