#include "vulkan_shader.h"

#include <ranges>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_reflect.hpp>

#include "renderer/pipeline/shader_compiler.h"
#include "renderer/pipeline/vulkan_pipeline.h"
#include "renderer/vulkan_buffer.h"
#include "renderer/vulkan_image.h"
#include "toki/resources/loaders/text_loader.h"
#include "vulkan/vulkan_core.h"
#include "vulkan_sampler.h"

namespace Toki {

VulkanShader::VulkanShader(const ShaderConfig& config) : Shader(config) {
    ShaderBinaries binaries;
    binaries.reserve(config.shaderStages.size());

    for (const auto& [stage, source] : m_config.shaderStages) {
        std::string shaderSource;

        if (std::holds_alternative<std::filesystem::path>(source)) {
            shaderSource = TextLoader::readTextFile(std::get<std::filesystem::path>(source)).value();
        } else {
            shaderSource = std::get<std::string>(source);
        }

        binaries.emplace_back(ShaderBinary{ .shaderStage = stage, .spirv = ShaderCompiler::compileShader("", ShaderSource{ stage, shaderSource }) });
    }

    if (std::holds_alternative<GraphicsShaderOptions>(config.options)) {
        m_pipeline = VulkanPipeline::create(binaries, std::get<GraphicsShaderOptions>(config.options));
        return;
    }

    std::unreachable();
}

VulkanShader::~VulkanShader() {
    m_pipeline.reset();
}

VkPipeline VulkanShader::getPipeline() const {
    return m_pipeline->getPipeline();
}

VkPipelineLayout VulkanShader::getPipelineLayout() const {
    return m_pipeline->getPipelineLayout();
}

uint32_t VulkanShader::getPushConstantStageFlags() const {
    return m_pipeline->getPushConstantShaderStageFlags();
}

const std::vector<VkDescriptorSet>& VulkanShader::getDestriptorSets() const {
    return m_pipeline->getDestriptorSets();
}

void VulkanShader::setUniforms(std::vector<Uniform> uniforms) {
    uint32_t bufferInfoSize = 0, imageInfoSize = 0;

    for (const auto& u : uniforms) {
        if (std::holds_alternative<Ref<UniformBuffer>>(u.uniform)) ++bufferInfoSize;
        if (std::holds_alternative<Ref<Texture>>(u.uniform) || std::holds_alternative<Ref<Sampler>>(u.uniform)) ++imageInfoSize;
    }

    std::vector<VkDescriptorBufferInfo> descriptorBufferInfos(bufferInfoSize);
    std::vector<VkDescriptorImageInfo> descriptorImageInfos(imageInfoSize);

    std::vector<VkWriteDescriptorSet> writes;
    writes.reserve(imageInfoSize + bufferInfoSize);

    uint32_t bufferInfoIndex = 0, imageInfoIndex = 0;

    for (const auto& u : uniforms) {
        if (std::holds_alternative<Ref<UniformBuffer>>(u.uniform)) {
            auto uniform = std::get<Ref<UniformBuffer>>(u.uniform);

            VkDescriptorBufferInfo& descriptorBufferInfo = descriptorBufferInfos[bufferInfoIndex];
            descriptorBufferInfo = {};
            descriptorBufferInfo.buffer = (VkBuffer) ((VulkanUniformBuffer*) uniform.get())->getHandle();
            descriptorBufferInfo.offset = 0;
            descriptorBufferInfo.range = uniform->getSize();

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = m_pipeline->getDestriptorSets()[u.setIndex];
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSet.pBufferInfo = &descriptorBufferInfos[bufferInfoIndex++];
            writeDescriptorSet.dstArrayElement = u.arrayElementIndex;
            writeDescriptorSet.dstBinding = u.binding;
            writes.emplace_back(writeDescriptorSet);
        }

        if (std::holds_alternative<Ref<Texture>>(u.uniform)) {
            auto texture = std::get<Ref<Texture>>(u.uniform);
            auto optionalSampler = texture->getOptionalSampler();

            VkDescriptorImageInfo& descriptorImageInfo = descriptorImageInfos[imageInfoIndex];
            descriptorImageInfo = {};
            descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptorImageInfo.imageView = (VkImageView) ((VulkanImage*) texture.get())->getImageView();
            descriptorImageInfo.sampler = VK_NULL_HANDLE;

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = m_pipeline->getDestriptorSets()[u.setIndex];
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            writeDescriptorSet.pImageInfo = &descriptorImageInfos[imageInfoIndex++];
            writeDescriptorSet.dstArrayElement = u.arrayElementIndex;
            writeDescriptorSet.dstBinding = u.binding;

            if (optionalSampler) {
                descriptorImageInfo.sampler = ((VulkanSampler*) optionalSampler.get())->getSampler();
                writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            }

            writes.emplace_back(writeDescriptorSet);
        }

        if (std::holds_alternative<Ref<Sampler>>(u.uniform)) {
            auto sampler = std::get<Ref<Sampler>>(u.uniform);

            VkDescriptorImageInfo& descriptorImageInfo = descriptorImageInfos[imageInfoIndex];
            descriptorImageInfo = {};
            descriptorImageInfo.sampler = ((VulkanSampler*) sampler.get())->getSampler();

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = m_pipeline->getDestriptorSets()[u.setIndex];
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            writeDescriptorSet.pImageInfo = &descriptorImageInfos[imageInfoIndex++];
            writeDescriptorSet.dstArrayElement = u.arrayElementIndex;
            writeDescriptorSet.dstBinding = u.binding;
            writes.emplace_back(writeDescriptorSet);
        }
    }

    vkUpdateDescriptorSets(s_context->device, writes.size(), writes.data(), 0, nullptr);
}

}  // namespace Toki
