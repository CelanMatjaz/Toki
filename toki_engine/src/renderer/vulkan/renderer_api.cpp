#include "renderer.h"
#include "renderer/vulkan/data/vulkan_shader.h"

namespace toki {

std::shared_ptr<Shader> VulkanRenderer::create_shader(const Shader::Config& config) const {
    return std::make_shared<VulkanShader>(m_context.get(), config);
}

}  // namespace toki
