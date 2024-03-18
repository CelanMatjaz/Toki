#include "compute_pipeline.h"

#include <toki/core/assert.h>

namespace Toki {

ComputePipeline::ComputePipeline(const ShaderBinaries& binaries, const ComputeShaderOptions& options) : m_options(options) {
    m_pipeline = create(binaries);
}

VkPipelineBindPoint ComputePipeline::getBindPoint() const {
    return VK_PIPELINE_BIND_POINT_COMPUTE;
}

VkPipeline ComputePipeline::create(const ShaderBinaries& binaries) {
    createLayoutFromBinaries(binaries);

    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
    createShaderStageCreateInfos(binaries, shaderStageCreateInfos);

    auto foundStage = std::find_if(shaderStageCreateInfos.begin(), shaderStageCreateInfos.end(), [](VkPipelineShaderStageCreateInfo& info) {
        return info.stage == VK_SHADER_STAGE_COMPUTE_BIT;
    });

    TK_ASSERT(foundStage != shaderStageCreateInfos.end(), "Compute shader stage not prvided");

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.stage = *foundStage;

    VkPipeline pipeline = VK_NULL_HANDLE;

    TK_ASSERT_VK_RESULT(
        vkCreateComputePipelines(s_context->device, VK_NULL_HANDLE, 1, &pipelineInfo, s_context->allocationCallbacks, &pipeline),
        "Could not create cumpute pipeline");

    return pipeline;
}

}  // namespace Toki
