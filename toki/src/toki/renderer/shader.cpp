#include "shader.h"

#include "renderer/vulkan_graphics_pipeline.h"

namespace Toki {

Ref<Shader> Shader::create(const ShaderConfig& config) {
    return createRef<VulkanGraphicsPipeline>(config);
}

Shader::Shader(const ShaderConfig& config) : m_config(config) {}

}  // namespace Toki