#pragma once

#include <toki/core.h>

#include "renderer_types.h"

namespace Toki {

[[nodiscard]] TkError create_swapchain(VulkanState* state, RendererWindow* renderer_window);
void destroy_swapchain(VulkanState* state, RendererWindow* renderer_window);

}  // namespace Toki
