#pragma once

#include <vulkan/vulkan.h>

#include "renderer/vulkan/renderer_state.h"

namespace toki {

VkShaderModule create_shader_module(Ref<RendererContext> ctx, std::vector<u32>& binary);

std::vector<u32> compile_shader(ShaderStage stage, std::string& source);

}  // namespace toki
