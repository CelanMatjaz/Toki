#include "toki/renderer/private/vulkan/vulkan_utils.h"

#include <toki/core/attributes.h>

namespace toki::renderer {

VkFormat map_color_format(ColorFormat format) {
	switch (format) {
		case ColorFormat::R8:
			return VK_FORMAT_R8_UNORM;
		case ColorFormat::RGBA8:
			return VK_FORMAT_R8G8B8A8_UNORM;
		default:
			TK_UNREACHABLE();
	}
}

}  // namespace toki::renderer
