#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace Toki {

struct RendererStateConfig {};

void renderer_initialize_state(const RendererStateConfig& config);
void renderer_destroy_state();

}  // namespace Toki
