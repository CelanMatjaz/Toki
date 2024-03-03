#pragma once

#include <filesystem>
#include <vector>

#include "toki/core/core.h"
#include "toki/renderer/renderer_types.h"

namespace Toki {

struct ShaderConfig {
    std::filesystem::path path{};
};

class Shader {
public:
    static Ref<Shader> create(const ShaderConfig& config);

    Shader(const Shader& other) = delete;
    Shader(Shader&& other) = delete;
    Shader& operator=(const Shader& other) = delete;
    Shader& operator=(const Shader&& other) = delete;
    virtual ~Shader() = default;

private:
    Shader() = default;
};

}  // namespace Toki
