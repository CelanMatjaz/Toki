#include "tkpch.h"
#include "vulkan_shader.h"
#include "core/assert.h"
#include "core/log.h"
#include "resources/file_loaders/shader_loader.h"
#include "unordered_map"
#include "vulkan_renderer.h"
#include "vulkan_framebuffer.h"
#include "platform/vulkan/backend/vulkan_utils.h"

namespace Toki {

    VulkanShader::VulkanShader(const ShaderConfig& config) : Shader(config) {
        switch (config.type) {
            case ShaderType::Graphics:
                bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                break;
        }

        reload();
    }

    VulkanShader::~VulkanShader() {
        vkDestroyPipelineLayout(VulkanRenderer::device(), pipelineLayout, nullptr);
    }

    void VulkanShader::bind() {
        std::vector<VkDescriptorSet> sets;
        for (const auto& [setIndex, set] : descriptorSets) {
            sets.push_back(set.getHandle());
        }

        VkCommandBuffer cmd = VulkanRenderer::commandBuffer();

        vkCmdBindDescriptorSets(cmd, bindPoint, pipelineLayout, 0, sets.size(), sets.data(), 0, nullptr);
        vkCmdBindPipeline(cmd, bindPoint, pipeline->getHandle());
    }

    void VulkanShader::reload() {
        switch (config.type) {
            case ShaderType::Graphics:
                initGraphics();
                break;
        }
    }

    void VulkanShader::initGraphics() {
        auto shaderCode = ShaderLoader::load(config.path);
        auto binaries = ShaderLoader::loadCompiledBinaries(config.path);

        // Assert stages in file
        TK_ASSERT(!binaries[ShaderStage::Vertex].empty(), "Vertex shader (#vert|#vertex) is required in shader file");
        TK_ASSERT(!binaries[ShaderStage::Fragment].empty(), "Fragment shader (#frag|#fragment|#pixel) is required in shader file");

        auto fragmentModuleSpirv = reflect(ShaderStage::Fragment, binaries[ShaderStage::Fragment]);
        auto vertexModuleSpirv = reflect(ShaderStage::Vertex, binaries[ShaderStage::Vertex]);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

        for (const auto& [set, bindings] : setLayoutBindings) {
            descriptorSets.emplace(set, bindings);
            descriptorSetLayouts.emplace_back(descriptorSets[set].getLayout());
        }

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();
        pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstants.size();

        TK_ASSERT_VK_RESULT(vkCreatePipelineLayout(VulkanRenderer::device(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout), "Could not create pipeline layout");

        PipelineConfig pipelineConfig{};
        pipelineConfig.type = config.type;
        pipelineConfig.layout = pipelineLayout;
        pipelineConfig.moduleSpirv[ShaderStage::Vertex] = vertexModuleSpirv;
        pipelineConfig.moduleSpirv[ShaderStage::Fragment] = fragmentModuleSpirv;
        pipelineConfig.inputAttributeDescriptions = mapAttributeDescriptions(config.attributeDescriptions);
        pipelineConfig.inputBindingDescriptions = mapBindingDescriptions(config.bindingDescriptions);
        pipelineConfig.renderPass = ((VulkanFramebuffer*) config.framebuffer.get())->getRenderPass();

        pipeline = Pipeline::create(pipelineConfig);

        vkDestroyShaderModule(VulkanRenderer::device(), pipelineConfig.moduleSpirv[ShaderStage::Vertex], nullptr);
        vkDestroyShaderModule(VulkanRenderer::device(), pipelineConfig.moduleSpirv[ShaderStage::Fragment], nullptr);
    }

    VkShaderModule VulkanShader::reflect(ShaderStage stage, const std::vector<uint32_t> spirv) {
        spirv_cross::Compiler compiler(spirv);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        getDescriptorSetBindings(stage, compiler, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        getDescriptorSetBindings(stage, compiler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        getPushConstants(stage, compiler);

        auto stageFlag = mapShaderStage(stage);

        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = spirv.size() * 4;
        shaderModuleCreateInfo.pCode = spirv.data();

        VkShaderModule shaderModule;
        TK_ASSERT_VK_RESULT(vkCreateShaderModule(VulkanRenderer::device(), &shaderModuleCreateInfo, nullptr, &shaderModule), "Could not create shader module");

        return shaderModule;
    }

    void VulkanShader::setUniform(Ref<UniformBuffer> uniformBuffer, uint32_t binding, uint32_t set, uint32_t index) {
        descriptorSets[set].setUniformBuffer((VulkanUniformBuffer*) uniformBuffer.get(), binding, index);
    }

    void VulkanShader::setTexture(Ref<Texture> texture, uint32_t binding, uint32_t set, uint32_t index) {
        descriptorSets[set].setTexture((VulkanTexture*) texture.get(), binding, index);
    }

    VkDescriptorSet VulkanShader::getSet(uint32_t set) {
        TK_ASSERT(descriptorSets.contains(set), "No desciptor set exists for stage");
        return descriptorSets[set].getHandle();
    }

    VkPipelineBindPoint VulkanShader::getPipelineBindPoint() {
        switch (config.type) {
            case ShaderType::Graphics: return VK_PIPELINE_BIND_POINT_GRAPHICS;
        }
    }

    shaderc_shader_kind VulkanShader::getShadercShaderKind(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex: return shaderc_shader_kind::shaderc_glsl_vertex_shader;
            case ShaderStage::Fragment: return shaderc_shader_kind::shaderc_glsl_fragment_shader;
        }

        return shaderc_shader_kind::shaderc_glsl_infer_from_source;
    }

    void VulkanShader::getDescriptorSetBindings(ShaderStage stage, spirv_cross::Compiler& compiler, VkDescriptorType descriptorType) {
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();
        spirv_cross::SmallVector<spirv_cross::Resource> resourceArray;

        VkShaderStageFlagBits stageFlag = mapShaderStage(stage);

        switch (descriptorType) {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                resourceArray = resources.uniform_buffers;
                break;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                resourceArray = resources.sampled_images;
                break;
            default:
                TK_ASSERT(false, std::format("Decriptor type {} is not supported\n", (int) descriptorType));
                return;
        }

        for (const auto& resource : resourceArray) {
            const auto& type = compiler.get_type(resource.base_type_id);
            const auto& typeArray = compiler.get_type(resource.type_id);

            uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            if (!setLayoutBindings.contains(set)) {
                setLayoutBindings[set] = {};
            }

            VkDescriptorSetLayoutBinding binding{};
            binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            binding.descriptorCount = typeArray.array.size() == 0 ? 1 : typeArray.array[0];
            binding.descriptorType = descriptorType;
            binding.stageFlags = stageFlag;

            if (setLayoutBindings[set].size() <= binding.binding) {
                setLayoutBindings[set].resize(binding.binding + 1);
                setLayoutBindings[set][binding.binding] = binding;
            }
            else {
                TK_ASSERT(setLayoutBindings[set][binding.binding].descriptorType == binding.descriptorType, std::format("Descriptor binding {} descriptor types do not match in all shader stages\n\tShader file: {}", binding.binding, config.path.string()));
                TK_ASSERT(setLayoutBindings[set][binding.binding].descriptorCount == binding.descriptorCount, std::format("Descriptor binding {} descriptor counts do not match in all shader stages\n\tShader file: {}", binding.binding, config.path.string()));

                setLayoutBindings[set][binding.binding].stageFlags |= stageFlag;

            }
        }
    }

    void VulkanShader::getPushConstants(ShaderStage stage, spirv_cross::Compiler& compiler) {
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();
        std::vector<VkPushConstantRange> constants;
        VkShaderStageFlagBits stageFlag = mapShaderStage(stage);

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(VulkanRenderer::physicalDevice(), &props);

        for (const auto& constant : resources.push_constant_buffers) {
            const auto& elementType = compiler.get_type(constant.base_type_id);

            VkPushConstantRange range{};
            range.offset = compiler.get_decoration(constant.id, spv::DecorationOffset);
            range.size = compiler.get_declared_struct_size(elementType);
            range.stageFlags = stageFlag;

            contantStageFlags = contantStageFlags | stageFlag;

            TK_ASSERT(range.size <= props.limits.maxPushConstantsSize, std::format("Push constant size is too big and not supported by the GPU ({}), max size: {}", range.size, props.limits.maxPushConstantsSize));

            pushConstants.emplace_back(range);
        }
    }

    std::vector<VkVertexInputBindingDescription> VulkanShader::mapBindingDescriptions(std::vector<VertexBindingDescription> bindingDescriptions) {
        std::vector<VkVertexInputBindingDescription> descriptions(bindingDescriptions.size());

        for (uint32_t i = 0; i < descriptions.size(); ++i) {
            descriptions[i].binding = bindingDescriptions[i].binding;
            descriptions[i].inputRate = (VkVertexInputRate) std::to_underlying(bindingDescriptions[i].inputRate);
            descriptions[i].stride = bindingDescriptions[i].stride;
        }

        return descriptions;
    }

    std::vector<VkVertexInputAttributeDescription> VulkanShader::mapAttributeDescriptions(std::vector<VertexAttributeDescription> attributeDescriptions) {
        std::vector<VkVertexInputAttributeDescription> descriptions(attributeDescriptions.size());

        for (uint32_t i = 0; i < descriptions.size(); ++i) {
            descriptions[i].binding = attributeDescriptions[i].binding;
            descriptions[i].format = VulkanUtils::mapVertexFormat(attributeDescriptions[i].format);
            descriptions[i].location = attributeDescriptions[i].location;
            descriptions[i].offset = attributeDescriptions[i].offset;
        }

        return descriptions;
    }

    VkShaderStageFlagBits VulkanShader::mapShaderStage(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
        }
    }

}