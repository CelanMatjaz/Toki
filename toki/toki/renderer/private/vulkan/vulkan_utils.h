#pragma once

#include <toki/core/core.h>
#include <vulkan/vulkan.h>

#include "toki/renderer/frontend/renderer_types.h"

namespace toki {

template <>
inline VkExtent2D convert_to<VkExtent2D>(const Vec2u32& vec) {
	return VkExtent2D{ vec.x, vec.y };
}

namespace renderer {

VkFormat map_color_format(ColorFormat format);

}  // namespace renderer

}  // namespace toki
