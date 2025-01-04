#pragma once

#include <vulkan/vulkan.h>

#include "core/core.h"
#include "core/macros.h"
#include "renderer/shader.h"
#include "renderer/vulkan/renderer_state.h"

namespace toki {

class VulkanShader : public Shader {
public:
    VulkanShader(RendererContext ctx, const Config& config);
    VulkanShader() = delete;
    ~VulkanShader();

    DELETE_COPY(VulkanShader);
    DELETE_MOVE(VulkanShader);

private:
    VkPipeline m_pipeline{};

    static VkShaderModule create_shader_module(RendererContext ctx, std::vector<u32>& binary);
};

}  // namespace toki
