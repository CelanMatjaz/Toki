#include "tkpch.h"
#include "shader.h"
#include "renderer.h"

#include "platform/vulkan/vulkan_shader.h"

namespace Toki {

    Ref<Shader> Shader::create(const ShaderConfig& config) {
        return createRef<VulkanShader>(config);
    }

    Shader::Shader(const ShaderConfig& config) : config{ std::move(config) } {}

}