#include "shader_config_loader.h"

#include <utility>

#include "core/assert.h"

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>

#include "shader_config_parsers.h"

namespace toki {

namespace configs {

ShaderConfig load_shader_config(std::filesystem::path path) {
    try {
        return YAML::LoadFile(path.string()).as<ShaderConfig>();
    } catch (std::exception e) {
        TK_ASSERT(false, "Error loading shader config: {}", e.what());
        std::unreachable();
    }
}

}  // namespace configs

}  // namespace toki
