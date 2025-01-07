#pragma once

#include "renderer/vulkan/vulkan_context.h"
#include "renderer/vulkan/vulkan_types.h"

namespace toki {

struct create_framebuffer_config {
    std::vector<render_target> render_targets;
};

vulkan_framebuffer vulkan_framebuffer_create(ref<renderer_context> ctx, const create_framebuffer_config& config);
void vulkan_framebuffer_destroy(ref<renderer_context> ctx, vulkan_framebuffer& framebuffer);

void vulkan_framebuffer_resize(ref<renderer_context> ctx, VkExtent3D extent, vulkan_framebuffer& framebuffer);

}  // namespace toki
