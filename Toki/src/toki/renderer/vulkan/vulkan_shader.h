#pragma once

#include "vulkan_renderer.h"
#include "vulkan_pipeline.h"

namespace Toki {

    class VulkanShader {
    public:
        ~VulkanShader();

        static std::unique_ptr<VulkanShader> create(std::string vertShaderPath, std::string fragShaderPath, VulkanPipeline::PipelineConfig pipelineConfig);

        vk::ShaderModule getVertShaderModule() const { return vertShaderModule; }
        vk::ShaderModule getFragShaderModule() const { return fragShaderModule; }
        vk::Pipeline getPipeline() const { return pipeline; }

        void recreatePipeline();

    private:
        VulkanShader() = default;

        void cleanup();

        static std::vector<char> loadShaderCode(const std::string& filePath);
        static vk::ShaderModule createShaderModule(const std::string& filePath);

        vk::ShaderModule vertShaderModule;
        vk::ShaderModule fragShaderModule;
        VulkanPipeline::PipelineConfig pipelineConfig;
        vk::Pipeline pipeline;
    };

}