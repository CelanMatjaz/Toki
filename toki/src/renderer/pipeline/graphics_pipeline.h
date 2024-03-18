#pragma once

#include "renderer/pipeline/vulkan_pipeline.h"
#include "toki/renderer/shader.h"

namespace Toki {

class GraphicsPipeline : public VulkanPipeline {
public:
    GraphicsPipeline() = default;
    GraphicsPipeline(const ShaderBinaries& binaries, const GraphicsShaderOptions& options);
    virtual ~GraphicsPipeline() = default;

    virtual VkPipelineBindPoint getBindPoint() const override;
    virtual VkPipeline create(const ShaderBinaries& binaries) override;

private:
    GraphicsShaderOptions m_options;
};

}  // namespace Toki
