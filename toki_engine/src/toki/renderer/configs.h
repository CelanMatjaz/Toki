#pragma once

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

struct TextureCreateConfig {
    glm::u32vec2 size;
};

struct BindVertexBuffersConfig {
    std::vector<Handle> handles;
    u32 first_binding = 0;
};

}  // namespace toki
