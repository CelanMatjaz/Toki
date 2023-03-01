#include "vulkan_shader.h"

namespace Toki {

    VulkanShader::~VulkanShader() {}

    // REVIEW: maybe don't save whole shader code, but load it everytime it's needed?
    std::unique_ptr<VulkanShader> VulkanShader::create(std::string vertShaderPath, std::string fragShaderPath, VulkanPipeline::PipelineConfig pipelineConfig) {
        std::unique_ptr<VulkanShader> shader(new VulkanShader());

        shader->vertShaderModule = VulkanShader::createShaderModule(vertShaderPath);
        shader->fragShaderModule = VulkanShader::createShaderModule(fragShaderPath);

        pipelineConfig.vertShaderModule = &shader->vertShaderModule;
        pipelineConfig.fragShaderModule = &shader->fragShaderModule;
        shader->pipelineConfig = pipelineConfig;
        shader->recreatePipeline();

        return shader;
    }

    void VulkanShader::recreatePipeline() {
        VulkanRenderer::getDevice().waitIdle();
        VulkanRenderer::getDevice().destroyPipeline(pipeline);
        pipeline = VulkanPipeline::createPipeline(pipelineConfig);
    }

    void VulkanShader::cleanup() {
        vk::Device device = VulkanRenderer::getDevice();
        device.destroyShaderModule(vertShaderModule);
        device.destroyShaderModule(fragShaderModule);
        device.destroyPipeline(pipeline);
    }

    std::vector<char> VulkanShader::loadShaderCode(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);
        uint32_t fileSize = file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    vk::ShaderModule VulkanShader::createShaderModule(const std::string& filePath) {
        auto data = loadShaderCode(filePath);

        vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.codeSize = data.size();
        shaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(data.data());

        vk::ShaderModule shaderModule;
        TK_ASSERT(VulkanRenderer::getDevice().createShaderModule(&shaderModuleCreateInfo, nullptr, &shaderModule) == vk::Result::eSuccess);
        return shaderModule;
    }
}