#pragma once

#include <filesystem>
#include <vector>

#include "renderer/renderer_types.h"
#include "resources/configs/shader_config_loader.h"

namespace toki {

struct ShaderCreateConfig {
    configs::ShaderConfig config;
    Handle framebuffer_handle;
};

struct BufferCreateConfig {
    u32 size{};
    BufferType type{};
    BufferUsage usage{};
};

struct FramebufferCreateConfig {
    std::vector<RenderTarget> render_targets;
};

}  // namespace toki
