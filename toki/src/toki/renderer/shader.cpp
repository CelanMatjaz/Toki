#include "shader.h"

#include "renderer/vulkan_shader.h"

namespace Toki {

Ref<Shader> Shader::create(const ShaderConfig& config) {
    return createRef<VulkanShader>(config);
}

Shader::Shader(const ShaderConfig& config) : m_config(config) {}

}  // namespace Toki
