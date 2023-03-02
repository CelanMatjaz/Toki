#include "vulkan_shader.h"

namespace Toki {

    VulkanShader::~VulkanShader() {}

    std::unique_ptr<VulkanShader> VulkanShader::create(VulkanPipeline::PipelineConfig pipelineConfig) {
        std::unique_ptr<VulkanShader> shader(new VulkanShader());
        shader->pipelineConfig = pipelineConfig;
        shader->pipelineIndex = VulkanPipeline::createPipeline(pipelineConfig);
        return shader;
    }

    void VulkanShader::bind(vk::CommandBuffer cmd) {
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, VulkanPipeline::getPipeline(pipelineIndex));
    }
}