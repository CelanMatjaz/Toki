#pragma once

#include "renderer/renderer_types.h"
#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_types.h"

namespace toki {

struct create_graphics_pipeline_config {
    std::filesystem::path vertex_shader_path;
    std::filesystem::path fragment_shader_path;
    std::vector<render_target> render_targets;
};

vulkan_graphics_pipeline vulkan_graphics_pipeline_create(ref<renderer_context> ctx, const create_graphics_pipeline_config& config);
void vulkan_graphics_pipeline_destroy(ref<renderer_context> ctx, vulkan_graphics_pipeline& pipeline);

}  // namespace toki
