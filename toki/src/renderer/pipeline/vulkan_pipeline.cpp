#include "vulkan_pipeline.h"

#include <algorithm>
#include <execution>
#include <spirv_cross/spirv_reflect.hpp>

#include "renderer/pipeline/graphics_pipeline.h"
#include "renderer/pipeline/shader_compiler.h"
#include "renderer/vulkan_utils.h"
#include "toki/core/assert.h"

namespace Toki {

Ref<VulkanPipeline> VulkanPipeline::create(const ShaderBinaries& binaries, const GraphicsShaderOptions& options) {
    return createRef<GraphicsPipeline>(binaries, options);
}

VulkanPipeline::~VulkanPipeline() {
    destroy();
}

void VulkanPipeline::destroy() {
    if (m_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(s_context->device, m_pipeline, s_context->allocationCallbacks);
        m_pipeline = VK_NULL_HANDLE;
    }

    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(s_context->device, m_pipelineLayout, s_context->allocationCallbacks);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    for (auto layout : m_descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(s_context->device, layout, s_context->allocationCallbacks);
    }
}

VkPipeline VulkanPipeline::getPipeline() const {
    return m_pipeline;
}

VkPipelineLayout VulkanPipeline::getPipelineLayout() const {
    return m_pipelineLayout;
}

VkShaderStageFlagBits VulkanPipeline::getPushConstantShaderStageFlags() const {
    return (VkShaderStageFlagBits) m_pushConstantStageFlags;
}

const std::vector<VkDescriptorSet>& VulkanPipeline::getDestriptorSets() const {
    return m_descriptorSets;
}

using SpirvResources = spirv_cross::SmallVector<spirv_cross::Resource>;

using SetBindings = std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>;

void reflectConstants(ShaderStage stage, spirv_cross::Compiler& compiler, std::vector<VkPushConstantRange>& pushConstantRanges);
void reflectDescriptors(ShaderStage stage, spirv_cross::Compiler& compiler, SetBindings& setBindings);

void VulkanPipeline::createLayoutFromBinaries(const ShaderBinaries& binaries) {
    std::vector<VkPushConstantRange> pushConstantRanges;

    SetBindings setBindings;

    for (const auto& [shaderStage, spirv] : binaries) {
        spirv_cross::Compiler compiler(spirv);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        reflectConstants(shaderStage, compiler, pushConstantRanges);
        reflectDescriptors(shaderStage, compiler, setBindings);

        // TODO: check max push constant size for physical device
    }

    for (const auto& c : pushConstantRanges) {
        m_pushConstantStageFlags |= c.stageFlags;
    }

    m_descriptorSetLayouts.resize(setBindings.size());

    for (const auto& [set, bindings] : setBindings) {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        TK_ASSERT_VK_RESULT(
            vkCreateDescriptorSetLayout(
                s_context->device, &descriptorSetLayoutCreateInfo, s_context->allocationCallbacks, &m_descriptorSetLayouts[set]),
            "Could not create descriptor set layout");
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRanges.size();
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
    pipelineLayoutCreateInfo.setLayoutCount = m_descriptorSetLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = m_descriptorSetLayouts.data();

    TK_ASSERT_VK_RESULT(
        vkCreatePipelineLayout(s_context->device, &pipelineLayoutCreateInfo, s_context->allocationCallbacks, &m_pipelineLayout),
        "Could not create pipeline layout");

    m_descriptorSets.resize(m_descriptorSetLayouts.size());

    if (m_descriptorSets.size() > 0) {
        allocateDescriptorSets(m_descriptorSetLayouts);
    }
}

void VulkanPipeline::allocateDescriptorSets(std::vector<VkDescriptorSetLayout>& layouts) {
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = s_context->descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = layouts.size();
    descriptorSetAllocateInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(layouts.size());

    if (m_descriptorSets.size() == 0) {
        return;
    }

    TK_ASSERT_VK_RESULT(
        vkAllocateDescriptorSets(s_context->device, &descriptorSetAllocateInfo, m_descriptorSets.data()), "Could not allocate descriptor sets");
}

void VulkanPipeline::createShaderStageCreateInfos(
    const ShaderBinaries& binaries, std::vector<VkPipelineShaderStageCreateInfo>& shaderStageCreateInfos) {
    for (uint32_t i = 0; i < binaries.size(); ++i) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = binaries[i].spirv.size() * 4;
        shaderModuleCreateInfo.pCode = binaries[i].spirv.data();

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.pName = "main";
        shaderStageCreateInfo.stage = VulkanUtils::mapShaderStage(binaries[i].shaderStage);

        TK_ASSERT_VK_RESULT(
            vkCreateShaderModule(s_context->device, &shaderModuleCreateInfo, s_context->allocationCallbacks, &shaderStageCreateInfo.module),
            "Could not create shader module");

        shaderStageCreateInfos.emplace_back(shaderStageCreateInfo);
    }
}

void reflectConstants(ShaderStage stage, spirv_cross::Compiler& compiler, std::vector<VkPushConstantRange>& pushConstants) {
    for (const auto& constant : compiler.get_shader_resources().push_constant_buffers) {
        auto elementType = compiler.get_type(constant.base_type_id);

        VkPushConstantRange range{};
        range.size = compiler.get_declared_struct_size(elementType);
        range.offset = compiler.get_decoration(constant.id, spv::DecorationOffset);
        range.stageFlags = VulkanUtils::mapShaderStage(stage);

        auto result = std::find_if(
            pushConstants.begin(), pushConstants.end(), [r = range](VkPushConstantRange constant) { return r.offset == constant.offset; });

        if (result != pushConstants.end()) {
            result->stageFlags |= range.stageFlags;
            continue;
        }

        pushConstants.emplace_back(range);
    }
}

void reflectDescriptors(ShaderStage stage, spirv_cross::Compiler& compiler, SetBindings& setBindings) {
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    struct ResourceDectriptorType {
        VkDescriptorType descriptorType;
        spirv_cross::SmallVector<spirv_cross::Resource> resources;
    };

    std::vector<ResourceDectriptorType> resourceArrays = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, resources.uniform_buffers }, { VK_DESCRIPTOR_TYPE_SAMPLER, resources.separate_samplers },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, resources.separate_images },  { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.sampled_images },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resources.storage_buffers }, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, resources.storage_images },
    };

    for (const auto& [descriptorType, resources] : resourceArrays) {
        for (const auto& resource : resources) {
            const auto& type = compiler.get_type(resource.base_type_id);
            const auto& typeArray = compiler.get_type(resource.type_id);

            uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

            VkDescriptorSetLayoutBinding binding{};
            binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            binding.descriptorCount = typeArray.array.size() == 0 ? 1 : typeArray.array[0];
            binding.descriptorType = descriptorType;
            binding.stageFlags = VulkanUtils::mapShaderStage(stage);

            auto result = std::find_if(setBindings[set].begin(), setBindings[set].end(), [b = binding](VkDescriptorSetLayoutBinding binding) {
                return b.binding == binding.binding;
            });

            if (result != setBindings[set].end()) {
                result->stageFlags |= binding.stageFlags;
                continue;
            }

            setBindings[set].emplace_back(binding);
        }
    }
}

}  // namespace Toki
