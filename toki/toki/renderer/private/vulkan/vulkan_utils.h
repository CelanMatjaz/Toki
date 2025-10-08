#pragma once

#include <toki/core/core.h>
#include <toki/renderer/errors.h>
#include <toki/renderer/frontend/renderer_types.h>
#include <toki/renderer/private/vulkan/vulkan_state.h>
#include <toki/renderer/private/vulkan/vulkan_types.h>
#include <vulkan/vulkan.h>

namespace toki {

template <>
inline VkExtent2D convert_to<VkExtent2D>(const Vec2u32& vec) {
	return VkExtent2D{ vec.x, vec.y };
}

namespace renderer {

toki::Expected<TempDynamicArray<toki::byte>, RendererErrors> compile_shader(ShaderStageFlags stage, StringView source);
VkShaderModule create_shader_module(const VulkanState& state, Span<toki::byte> spirv);

VkFormat map_color_format(ColorFormat format);

}  // namespace renderer

}  // namespace toki
