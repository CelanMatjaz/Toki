#pragma once

#include <vulkan/vulkan.h>

#include "core/core.h"
#include "core/macros.h"
#include "renderer/shader.h"
#include "renderer/vulkan/renderer_state.h"

namespace toki {

class VulkanShader : public Shader {
public:
    VulkanShader(Ref<RendererContext> ctx, const Shader::Config& config);
    VulkanShader() = delete;
    ~VulkanShader();

    inline static Ref<RendererContext> s_context;

    // DELETE_COPY(VulkanShader)
    // DELETE_MOVE(VulkanShader)

private:
    VkPipeline m_pipeline{};
    VkPipelineLayout m_pipelineLayout{};

    static VkShaderModule create_shader_module(Ref<RendererContext> ctx, std::vector<u32>& binary);
};

}  // namespace toki
