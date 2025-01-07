#pragma once

#include <filesystem>
#include <vector>

#include "renderer/renderer_types.h"

namespace toki {

struct shader_create_config {
    std::filesystem::path vertex_shader_path{};
    std::filesystem::path fragment_shader_path{};
    Handle framebuffer_handle;
};

struct buffer_create_config {
    u32 size{};
    BufferType type{};
    BufferUsage usage{};
};

struct framebuffer_create_config {
    std::vector<RenderTarget> render_targets;
};

}  // namespace toki
