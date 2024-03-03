#pragma once

#include "renderer/vulkan_types.h"
#include "toki/core/window.h"

namespace Toki {

namespace VulkanUtils {

VkSurfaceKHR createSurface(Ref<VulkanContext> context, Ref<Window> window);

}

}  // namespace Toki