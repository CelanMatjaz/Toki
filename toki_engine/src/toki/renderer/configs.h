#pragma once

#include <filesystem>
#include <vector>

#include "renderer/renderer_types.h"

namespace toki {

struct shader_create_config {
    std::filesystem::path vertex_shader_path{};
    std::filesystem::path fragment_shader_path{};
    std::vector<render_target> render_targets;
};

struct buffer_create_config {
    u32 size{};
    buffer_type type{};
    buffer_usage usage{};
};

}  // namespace toki
