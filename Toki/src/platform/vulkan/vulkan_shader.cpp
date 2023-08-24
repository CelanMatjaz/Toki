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
        for (const auto& layout : descriptorSetLayouts) {
            vkDestroyDescriptorSetLayout(VulkanRenderer::device(), layout, nullptr);
        }

        vkDestroyPipelineLayout(VulkanRenderer::device(), pipelineLayout, nullptr);
    }

    void VulkanShader::bind() {
        vkCmdBindPipeline(VulkanRenderer::commandBuffer(), bindPoint, pipeline->getHandle());
    }

    void VulkanShader::reload() {
        switch (config.type) {
            case ShaderType::Graphics:
                initGraphics();
                break;
        }
    }

    void VulkanShader::initGraphics() {
        auto shaderCode = ShaderLoader::loadRaw(config.path);

        constexpr uint32_t vertexStageIndex = std::to_underlying(ShaderStage::Vertex);
        constexpr uint32_t fragmentStageIndex = std::to_underlying(ShaderStage::Fragment);

        std::cout << "Loading shader file " << std::filesystem::absolute(config.path) << '\n';

        // Assert stages in file
        TK_ASSERT(!shaderCode[vertexStageIndex].empty(), "Vertex shader (#vert|#vertex) is required in shader file");
        TK_ASSERT(!shaderCode[fragmentStageIndex].empty(), "Fragment shader (#frag|#fragment|#pixel) is required in shader file");

        std::vector<uint32_t> moduleConfigs[std::to_underlying(ShaderStage::SHADER_STAGE_MAX_ENUM)];

        auto fragmentModuleSpirv = reflect(ShaderStage::Fragment, shaderCode[fragmentStageIndex]);
        auto vertexModuleSpirv = reflect(ShaderStage::Vertex, shaderCode[vertexStageIndex]);

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
        pipelineConfig.moduleSpirv[vertexStageIndex] = vertexModuleSpirv;
        pipelineConfig.moduleSpirv[fragmentStageIndex] = fragmentModuleSpirv;
        pipelineConfig.inputAttributeDescriptions = mapAttributeDescriptions(config.attributeDescriptions);
        pipelineConfig.inputBindingDescriptions = mapBindingDescriptions(config.bindingDescriptions);
        pipelineConfig.renderPass = ((VulkanFramebuffer*) config.framebuffer.get())->getRenderPass();

        pipeline = Pipeline::create(pipelineConfig);

        vkDestroyShaderModule(VulkanRenderer::device(), pipelineConfig.moduleSpirv[vertexStageIndex], nullptr);
        vkDestroyShaderModule(VulkanRenderer::device(), pipelineConfig.moduleSpirv[fragmentStageIndex], nullptr);
    }

    VkShaderModule VulkanShader::reflect(ShaderStage stage, std::string_view shaderCode) {
        shaderc::Compiler spirvCompiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

    #ifdef NDEBUG
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
    #else
        options.SetOptimizationLevel(shaderc_optimization_level_zero);
    #endif

        options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);

        shaderc::SpvCompilationResult spirvModule = spirvCompiler.CompileGlslToSpv(shaderCode.data(), getShadercShaderKind(stage), "Shader", options);
        TK_ASSERT(spirvModule.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success, std::format("Error compiling shader code from file {}\n\t{}", std::filesystem::absolute(config.path).string(), spirvModule.GetErrorMessage()));

        std::vector<uint32_t> spirv(spirvModule.begin(), spirvModule.end());
        spirv_cross::Compiler compiler(spirv);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        auto uniforms = getDescriptorSetBindings(stage, compiler, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        auto samplers = getDescriptorSetBindings(stage, compiler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        pushConstants = getPushConstants(stage, compiler);

        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> sets;
        for (const auto& [set, uniformBindings] : uniforms) {
            if (!sets.contains(set))
                sets[set] = {};

            for (const auto& binding : uniformBindings) {
                sets[set].emplace_back(binding);
            }
        }

        for (const auto& [set, samplerBindings] : samplers) {
            if (!sets.contains(set))
                sets[set] = {};

            for (const auto& binding : samplerBindings) {
                sets[set].emplace_back(binding);
            }
        }

        std::vector<VkDescriptorSetLayout> layouts;

        for (const auto& [set, bindings] : sets) {
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.pBindings = bindings.data();
            layoutInfo.bindingCount = bindings.size();

            VkDescriptorSetLayout descriptorSetLayout;
            TK_ASSERT_VK_RESULT(vkCreateDescriptorSetLayout(VulkanRenderer::device(), &layoutInfo, nullptr, &descriptorSetLayout), std::format("Could not create descriptor set layout for set {}", set));
            layouts.emplace_back(descriptorSetLayout);
            descriptorSetLayouts.emplace_back(descriptorSetLayout);
        }

        descriptorSets[std::to_underlying(stage)].resize(layouts.size());

        if (layouts.size()) {
            VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
            descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorSetAllocateInfo.descriptorPool = VulkanRenderer::descriptorPool();
            descriptorSetAllocateInfo.descriptorSetCount = layouts.size();
            descriptorSetAllocateInfo.pSetLayouts = layouts.data();

            TK_ASSERT_VK_RESULT(vkAllocateDescriptorSets(VulkanRenderer::device(), &descriptorSetAllocateInfo, descriptorSets[std::to_underlying(stage)].data()), "Could not allocate descriptor sets");
        }

        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = spirv.size() * 4;
        shaderModuleCreateInfo.pCode = spirv.data();

        VkShaderModule shaderModule;
        TK_ASSERT_VK_RESULT(vkCreateShaderModule(VulkanRenderer::device(), &shaderModuleCreateInfo, nullptr, &shaderModule), "Could not create shader module");

        return shaderModule;
    }

    std::vector<VkDescriptorSet> VulkanShader::getDescriptorSets(ShaderStage stage) {
        std::vector<VkDescriptorSet> sets;

        for (const auto& set : descriptorSets[std::to_underlying(stage)]) {
            sets.emplace_back(set);
        }

        return std::move(sets);
    }

    VkPipelineBindPoint VulkanShader::getPipelineBindPoint() {
        switch (config.type) {
            case ShaderType::Graphics: return VK_PIPELINE_BIND_POINT_GRAPHICS;
        }
    }

    VkShaderStageFlagBits VulkanShader::getVulkanShaderStage(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }

    shaderc_shader_kind VulkanShader::getShadercShaderKind(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex: return shaderc_shader_kind::shaderc_glsl_vertex_shader;
            case ShaderStage::Fragment: return shaderc_shader_kind::shaderc_glsl_fragment_shader;
        }

        return shaderc_shader_kind::shaderc_glsl_infer_from_source;
    }

    std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> VulkanShader::getDescriptorSetBindings(ShaderStage stage, spirv_cross::Compiler& compiler, VkDescriptorType descriptorType) {
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();
        spirv_cross::SmallVector<spirv_cross::Resource> resourceArray;
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindings;
        VkShaderStageFlagBits stageFlag = getVulkanShaderStage(stage);

        switch (descriptorType) {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                resourceArray = resources.uniform_buffers;
                break;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                resourceArray = resources.sampled_images;
                break;
            default:
                return {};
        }

        for (const auto& resource : resourceArray) {
            const auto& type = compiler.get_type(resource.base_type_id);
            const auto& typeArray = compiler.get_type(resource.type_id);

            uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            if (!bindings.contains(set)) {
                bindings[set] = {};
            }

            VkDescriptorSetLayoutBinding binding{};
            binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            binding.descriptorCount = typeArray.array.size() == 0 ? 1 : typeArray.array[0];
            binding.descriptorType = descriptorType;
            binding.stageFlags = stageFlag;

            bindings[set].emplace_back(binding);
        }

        return bindings;
    }

    std::vector<VkPushConstantRange> VulkanShader::getPushConstants(ShaderStage stage, spirv_cross::Compiler& compiler) {
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();
        std::vector<VkPushConstantRange> constants;
        VkShaderStageFlagBits stageFlag = getVulkanShaderStage(stage);

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(VulkanRenderer::physicalDevice(), &props);

        for (const auto& constant : resources.push_constant_buffers) {
            const auto& elementType = compiler.get_type(constant.base_type_id);

            VkPushConstantRange range{};
            range.offset = compiler.get_decoration(constant.id, spv::DecorationOffset);
            range.size = compiler.get_declared_struct_size(elementType);
            range.stageFlags = stageFlag;

            TK_ASSERT(range.size <= props.limits.maxPushConstantsSize, std::format("Push constant size is too big and not supported by the GPU ({}), max size: {}", range.size, props.limits.maxPushConstantsSize));

            constants.emplace_back(range);
        }

        return constants;
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