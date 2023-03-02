#pragma once

#include "vulkan_renderer.h"
#include "vulkan_pipeline.h"

namespace Toki {

    class VulkanShader {
    public:
        ~VulkanShader();

        static std::unique_ptr<VulkanShader> create(VulkanPipeline::PipelineConfig pipelineConfig);

        void bind(vk::CommandBuffer cmd);

    private:
        VulkanShader() = default;

        VulkanPipeline::PipelineConfig pipelineConfig;
        uint32_t pipelineIndex;
    };

}