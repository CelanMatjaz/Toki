#include "vulkan_graphics_pipeline.h"

#include <ranges>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_reflect.hpp>

#include "renderer/vulkan_buffer.h"
#include "renderer/vulkan_image.h"
#include "renderer/vulkan_utils.h"
#include "toki/core/assert.h"
#include "toki/resources/loaders/text_loader.h"
#include "toki/resources/resource_utils.h"
#include "vulkan/vulkan_core.h"
#include "vulkan_sampler.h"

namespace Toki {

static std::vector<uint32_t> getCompiledBinary(const std::pair<ShaderStage, std::variant<std::string, std::filesystem::path>>& stage);
static std::vector<VkPushConstantRange> reflectConstants(ShaderStage, const std::vector<uint32_t>& spirv);
static std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> reflectDescriptors(
    const std::unordered_map<ShaderStage, std::vector<uint32_t>>& p);

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const ShaderConfig& config) : Shader(config) {
    create();
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
    destroy();
}

VkPipeline VulkanGraphicsPipeline::getPipeline() const {
    return m_pipeline;
}

VkPipelineLayout VulkanGraphicsPipeline::getPipelineLayout() const {
    return m_pipelineLayout;
}

uint32_t VulkanGraphicsPipeline::getPushConstantStageFlags() const {
    return m_pushConstantStageFlags;
}

const std::vector<VkDescriptorSet>& VulkanGraphicsPipeline::getDestriptorSets() const {
    return m_descriptorSets;
}

void VulkanGraphicsPipeline::setUniforms(std::vector<Uniform> uniforms) {
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
            writeDescriptorSet.dstSet = m_descriptorSets[u.setIndex];
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
            writeDescriptorSet.dstSet = m_descriptorSets[u.setIndex];
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
            writeDescriptorSet.dstSet = m_descriptorSets[u.setIndex];
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

void VulkanGraphicsPipeline::create() {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
    std::vector<VkShaderModule> shaderModules;
    shaderModules.reserve(m_config.shaderStages.size());

    std::vector<VkPushConstantRange> constants;

    std::unordered_map<ShaderStage, std::vector<uint32_t>> spirv;

    for (auto& pair : m_config.shaderStages) {
        auto& code = spirv[pair.first] = getCompiledBinary(pair);

        constants.append_range(reflectConstants(pair.first, code));

        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = code.size() * 4;
        shaderModuleCreateInfo.pCode = code.data();

        static uint32_t i = 0;

        VkShaderModule shaderModule = VK_NULL_HANDLE;

        TK_ASSERT_VK_RESULT(
            vkCreateShaderModule(s_context->device, &shaderModuleCreateInfo, s_context->allocationCallbacks, &shaderModule),
            "Could not create shader module");

        shaderModules.emplace_back(shaderModule);

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.module = shaderModules.back();
        shaderStageCreateInfo.pName = "main";

        switch (pair.first) {
            case ShaderStage::SHADER_STAGE_VERTEX:
                shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case ShaderStage::SHADER_STAGE_FRAGMENT:
                shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            default:
                std::unreachable();
        }

        shaderStageCreateInfos.emplace_back(shaderStageCreateInfo);

        ++i;
    }

    for (const auto& c : constants) {
        m_pushConstantStageFlags |= c.stageFlags;
    }

    VertexLayoutDescriptions layout = m_config.layoutDescriptions;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions(layout.bindingDescriptions.size());
    for (uint32_t i = 0; i < layout.bindingDescriptions.size(); ++i) {
        bindingDescriptions[i].stride = layout.bindingDescriptions[i].stride;
        bindingDescriptions[i].binding = layout.bindingDescriptions[i].binding;

        switch (layout.bindingDescriptions[i].inputRate) {
            case VertexInputRate::VERTEX_INPUT_RATE_VERTEX:
                bindingDescriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                break;
            case VertexInputRate::VERTEX_INPUT_RATE_INSTANCE:
                bindingDescriptions[i].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                break;
            default:
                std::unreachable();
        }
    }

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(layout.attributeDescriptions.size());
    for (uint32_t i = 0; i < layout.attributeDescriptions.size(); ++i) {
        attributeDescriptions[i].binding = layout.attributeDescriptions[i].binding;
        attributeDescriptions[i].offset = layout.attributeDescriptions[i].offset;
        attributeDescriptions[i].location = layout.attributeDescriptions[i].location;

        switch (layout.attributeDescriptions[i].format) {
            case VertexFormat::VERTEX_FORMAT_FLOAT1:
                attributeDescriptions[i].format = VK_FORMAT_R32_SFLOAT;
                break;
            case VertexFormat::VERTEX_FORMAT_FLOAT2:
                attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case VertexFormat::VERTEX_FORMAT_FLOAT3:
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case VertexFormat::VERTEX_FORMAT_FLOAT4:
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            default:
                std::unreachable();
        }
    }

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    switch (m_config.options.primitiveTopology) {
        case PrimitiveTopology::PointList:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case PrimitiveTopology::LineList:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case PrimitiveTopology::LineStrip:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case PrimitiveTopology::TriangleList:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case PrimitiveTopology::TriangleStrip:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case PrimitiveTopology::TriangleFan:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            break;
        case PrimitiveTopology::LineListWithAdjacency:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::LineStripWithAdjacency:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::TriangleListWithAdjacency:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::TriangleStripWithAdjacency:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::PatchList:
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            break;
        default:
            std::unreachable();
    }

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizerCreateInto{};
    rasterizerCreateInto.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInto.depthClampEnable = VK_FALSE;
    rasterizerCreateInto.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInto.lineWidth = 1.0f;
    rasterizerCreateInto.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerCreateInto.depthBiasEnable = VK_FALSE;
    rasterizerCreateInto.depthBiasConstantFactor = 0.0f;
    rasterizerCreateInto.depthBiasClamp = 0.0f;
    rasterizerCreateInto.depthBiasSlopeFactor = 0.0f;

    switch (m_config.options.polygonMode) {
        case PolygonMode::Fill:
            rasterizerCreateInto.polygonMode = VK_POLYGON_MODE_FILL;
            break;
        case PolygonMode::Line:
            rasterizerCreateInto.polygonMode = VK_POLYGON_MODE_LINE;
            break;
        case PolygonMode::Point:
            rasterizerCreateInto.polygonMode = VK_POLYGON_MODE_POINT;
            break;
        default:
            std::unreachable();
    }

    switch (m_config.options.cullMode) {
        case CullMode::None:
            rasterizerCreateInto.cullMode = VK_CULL_MODE_NONE;
            break;
        case CullMode::Front:
            rasterizerCreateInto.cullMode = VK_CULL_MODE_FRONT_BIT;
            break;
        case CullMode::Back:
            rasterizerCreateInto.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case CullMode::FrontAndBack:
            rasterizerCreateInto.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
            break;
        default:
            std::unreachable();
    }

    switch (m_config.options.frontFace) {
        case FrontFace::CounterClockwise:
            rasterizerCreateInto.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            break;
        case FrontFace::Clockwise:
            rasterizerCreateInto.frontFace = VK_FRONT_FACE_CLOCKWISE;
            break;
        default:
            std::unreachable();
    }

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCreateInfo.minSampleShading = 1.0f;
    multisampleStateCreateInfo.pSampleMask = nullptr;
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
    depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateCreateInfo.depthTestEnable = m_config.options.depthTest.enable ? VK_TRUE : VK_FALSE;
    depthStencilStateCreateInfo.depthWriteEnable = m_config.options.depthTest.write ? VK_TRUE : VK_FALSE;
    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;  // TODO: add a switch statement for this (m_config.options.depthTest.compareOp)
    depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.minDepthBounds = 0.0f;
    depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
    depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.front = {};
    depthStencilStateCreateInfo.back = {};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    uint32_t colorAttachmentCount = 0;
    for (const auto& attachment : m_config.attachments) {
        if (attachment.colorFormat == ColorFormat::COLOR_FORMAT_RGBA) {
            ++colorAttachmentCount;
        }
    }

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates(colorAttachmentCount, colorBlendAttachment);

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.attachmentCount = colorBlendAttachmentStates.size();
    colorBlendStateCreateInfo.pAttachments = colorBlendAttachmentStates.data();
    colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> states = { VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = states.data();
    dynamicState.dynamicStateCount = states.size();

    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(s_context->device, m_pipelineLayout, s_context->allocationCallbacks);
    }

    auto sets = reflectDescriptors(spirv);

    m_descriptorSetLayouts.resize(sets.size());

    for (const auto& [set, bindings] : sets) {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        TK_ASSERT_VK_RESULT(
            vkCreateDescriptorSetLayout(
                s_context->device, &descriptorSetLayoutCreateInfo, s_context->allocationCallbacks, &m_descriptorSetLayouts[set]),
            "Could not create descriptor set layout");
    }

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = s_context->descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = m_descriptorSetLayouts.size();
    descriptorSetAllocateInfo.pSetLayouts = m_descriptorSetLayouts.data();

    m_descriptorSets.resize(m_descriptorSetLayouts.size());
    TK_ASSERT_VK_RESULT(
        vkAllocateDescriptorSets(s_context->device, &descriptorSetAllocateInfo, m_descriptorSets.data()), "Could not allocate descriptor sets");

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = constants.size();
    pipelineLayoutInfo.pPushConstantRanges = constants.data();
    pipelineLayoutInfo.setLayoutCount = m_descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();

    TK_ASSERT_VK_RESULT(
        vkCreatePipelineLayout(s_context->device, &pipelineLayoutInfo, s_context->allocationCallbacks, &m_pipelineLayout),
        "Could not create pipeline layout");

    VkPipelineRenderingCreateInfoKHR renderingCreateInfo{};
    renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.viewMask = 0;
    renderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    std::vector<VkFormat> colorFormats;

    for (const auto& attachment : m_config.attachments) {
        if (attachment.presentable) {
            colorFormats.emplace_back(VK_FORMAT_B8G8R8A8_SRGB);
            continue;
        }

        switch (attachment.colorFormat) {
            case ColorFormat::COLOR_FORMAT_DEPTH:
                renderingCreateInfo.depthAttachmentFormat = VulkanUtils::mapFormat(attachment.colorFormat);
                break;
            case ColorFormat::COLOR_FORMAT_STENCIL:
                renderingCreateInfo.stencilAttachmentFormat = VulkanUtils::mapFormat(attachment.colorFormat);
                break;
            case ColorFormat::COLOR_FORMAT_DEPTH_STENCIL:
                renderingCreateInfo.depthAttachmentFormat = VulkanUtils::mapFormat(attachment.colorFormat);
                renderingCreateInfo.stencilAttachmentFormat = VulkanUtils::mapFormat(attachment.colorFormat);
                break;
            default:
                colorFormats.emplace_back(VulkanUtils::mapFormat(attachment.colorFormat));
        }
    }

    renderingCreateInfo.colorAttachmentCount = colorFormats.size();
    renderingCreateInfo.pColorAttachmentFormats = colorFormats.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &renderingCreateInfo;
    pipelineInfo.stageCount = shaderStageCreateInfos.size();
    pipelineInfo.pStages = shaderStageCreateInfos.data();
    pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizerCreateInto;
    pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
    pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;

    TK_ASSERT_VK_RESULT(
        vkCreateGraphicsPipelines(s_context->device, VK_NULL_HANDLE, 1, &pipelineInfo, s_context->allocationCallbacks, &m_pipeline),
        "Could not create graphics pipeline");

    for (auto& shaderModule : shaderModules) {
        vkDestroyShaderModule(s_context->device, shaderModule, s_context->allocationCallbacks);
    }
}

void VulkanGraphicsPipeline::destroy() {
    vkDestroyPipeline(s_context->device, m_pipeline, s_context->allocationCallbacks);
    vkDestroyPipelineLayout(s_context->device, m_pipelineLayout, s_context->allocationCallbacks);

    for (const auto& layout : m_descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(s_context->device, layout, s_context->allocationCallbacks);
    }
}

static std::vector<uint32_t> getCompiledBinary(const std::pair<ShaderStage, std::variant<std::string, std::filesystem::path>>& stage) {
    shaderc::Compiler spirvCompiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);

#ifdef TK_NDEBUG
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

    shaderc_shader_kind shaderKind;

    switch (stage.first) {
        case ShaderStage::SHADER_STAGE_VERTEX:
            shaderKind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
            break;
        case ShaderStage::SHADER_STAGE_FRAGMENT:
            shaderKind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
            break;
        default:
            std::unreachable();
    }

    std::expected<std::string, Toki::Error> fileSource;

    if (std::holds_alternative<std::filesystem::path>(stage.second)) {
        fileSource = TextLoader::readTextFile(std::get<std::filesystem::path>(stage.second));

        if (!fileSource.has_value()) {
            std::unreachable();
        }
    } else {
        fileSource = std::get<std::string>(stage.second);
    }

    shaderc::SpvCompilationResult spirvModule = spirvCompiler.CompileGlslToSpv(fileSource.value().c_str(), shaderKind, "", options);
    if (spirvModule.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
        std::println("ERROR MESSAGE:\n{}\n\nCOMPILATION STATUS: {}\n", spirvModule.GetErrorMessage(), (int) spirvModule.GetCompilationStatus());
    }
    TK_ASSERT(spirvModule.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success, "Shader compilation error");

    TK_ASSERT(spirvModule.end() - spirvModule.begin() > 0, "");
    return { spirvModule.begin(), spirvModule.end() };
}

static std::vector<VkPushConstantRange> reflectConstants(ShaderStage stage, const std::vector<uint32_t>& spirv) {
    spirv_cross::Compiler compiler(spirv);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    std::vector<VkPushConstantRange> constants;
    VkShaderStageFlagBits stageFlag = VulkanUtils::mapShaderStage(stage);

    for (const auto& constant : resources.push_constant_buffers) {
        const auto& elementType = compiler.get_type(constant.base_type_id);

        VkPushConstantRange range{};
        range.offset = compiler.get_decoration(constant.id, spv::DecorationOffset);
        range.size = compiler.get_declared_struct_size(elementType);
        range.stageFlags = stageFlag;

        constants.emplace_back(range);
    }

    return constants;
}

struct ResourceDectriptorType {
    VkDescriptorType descriptorType;
    spirv_cross::SmallVector<spirv_cross::Resource> resources;
};

static std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> reflectDescriptors(
    const std::unordered_map<ShaderStage, std::vector<uint32_t>>& p) {
    std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindings;

    for (const auto& [stage, spirv] : p) {
        spirv_cross::Compiler compiler(spirv);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();
        std::vector<ResourceDectriptorType> resourcesArrays = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, resources.uniform_buffers },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, resources.separate_images },
            { VK_DESCRIPTOR_TYPE_SAMPLER, resources.separate_samplers },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.sampled_images },
        };

        spirv_cross::SmallVector<spirv_cross::Resource> resourceArray;

        VkShaderStageFlagBits stageFlags = VulkanUtils::mapShaderStage(stage);

        for (auto& [descriptorType, resources] : resourcesArrays) {
            for (const auto& resource : resources) {
                const auto& type = compiler.get_type(resource.base_type_id);
                const auto& typeArray = compiler.get_type(resource.type_id);

                uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

                VkDescriptorSetLayoutBinding binding{};
                binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                binding.descriptorCount = typeArray.array.size() == 0 ? 1 : typeArray.array[0];
                binding.descriptorType = descriptorType;
                binding.stageFlags = stageFlags;

                if (auto result = std::ranges::find_if(
                        bindings[set].begin(),
                        bindings[set].end(),
                        [b = binding.binding](VkDescriptorSetLayoutBinding binding) { return b == binding.binding; });
                    result != bindings[set].end()) {
                    (*result).stageFlags |= binding.stageFlags;
                } else {
                    bindings[set].emplace_back(binding);
                }
            }
        }
    }

    return bindings;
}

}  // namespace Toki
