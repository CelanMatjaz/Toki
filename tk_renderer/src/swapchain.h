#pragma once

#include <toki/core.h>

#include "renderer_types.h"

namespace Toki {

TkError create_swapchain(VulkanState* state, RendererWindow* renderer_window);

}
