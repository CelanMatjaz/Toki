#pragma once

#include "renderer/pipeline/vulkan_pipeline.h"
#include "toki/renderer/shader.h"

namespace Toki {

class ComputePipeline : public VulkanPipeline {
public:
    ComputePipeline() = default;
    ComputePipeline(const ShaderBinaries& binaries, const ComputeShaderOptions& options);
    virtual ~ComputePipeline() = default;

    virtual VkPipelineBindPoint getBindPoint() const override;
    virtual VkPipeline create(const ShaderBinaries& binaries) override;

private:
    ComputeShaderOptions m_options;
};

}  // namespace Toki
