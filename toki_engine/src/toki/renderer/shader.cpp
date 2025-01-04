#include "shader.h"

#include <memory>

#include "renderer/vulkan/data/vulkan_shader.h"

namespace toki {

std::shared_ptr<Shader> Shader::create(const Config& config) {
    return std::make_shared<VulkanShader>(config);
}

}  // namespace toki
