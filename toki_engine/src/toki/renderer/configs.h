#pragma once

#include <vector>

#include "renderer/renderer_types.h"
#include "resources/configs/shader_config_loader.h"

namespace toki {

struct ShaderCreateConfig {
    configs::ShaderConfig config;
    Handle framebuffer_handle;
};

}  // namespace toki
