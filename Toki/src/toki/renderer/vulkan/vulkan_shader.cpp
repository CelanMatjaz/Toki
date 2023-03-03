#include "vulkan_shader.h"

namespace Toki {

    VulkanShader::~VulkanShader() {}

    std::unique_ptr<VulkanShader> VulkanShader::create(VulkanPipeline::PipelineConfig pipelineConfig) {
        std::unique_ptr<VulkanShader> shader(new VulkanShader());
        shader->pipelineConfig = pipelineConfig;
        shader->pipelineIndex = VulkanPipeline::createPipeline(pipelineConfig);
        return shader;
    }

    void VulkanShader::bind(VkCommandBuffer cmd) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanPipeline::getPipeline(pipelineIndex));
    }
}