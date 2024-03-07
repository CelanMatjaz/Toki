#include "vulkan_graphics_pipeline.h"

#include <shaderc/shaderc.hpp>

#include "toki/core/assert.h"
#include "toki/resources/loaders/text_loader.h"
#include "toki/resources/resource_utils.h"

namespace Toki {

static std::vector<uint32_t> getCompiledBinary(const std::pair<ShaderStage, std::filesystem::path>& stage);

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const ShaderConfig& config) : Shader(config) {
    create();
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
    vkDestroyPipeline(s_context->device, m_pipeline, s_context->allocationCallbacks);
}

VkPipeline VulkanGraphicsPipeline::getPipeline() const {
    return m_pipeline;
}

void VulkanGraphicsPipeline::create() {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
    std::vector<VkShaderModule> shaderModules;

    for (const auto& pair : m_config.shaderStagePaths) {
        auto code = getCompiledBinary(pair);

        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = code.size() * 4;
        shaderModuleCreateInfo.pCode = code.data();

        static uint32_t i = 0;

        VkShaderModule shaderModule;

        TK_ASSERT_VK_RESULT(
            vkCreateShaderModule(s_context->device, &shaderModuleCreateInfo, s_context->allocationCallbacks, &shaderModule),
            "Could not create shader module"
        );

        shaderModules.emplace_back(shaderModule);

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.module = shaderModules[i];
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
        }
    }

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(layout.attributeDescriptions.size());
    for (uint32_t i = 0; i < layout.bindingDescriptions.size(); ++i) {
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
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

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
    rasterizerCreateInto.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInto.lineWidth = 1.0f;
    rasterizerCreateInto.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInto.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerCreateInto.depthBiasEnable = VK_FALSE;
    rasterizerCreateInto.depthBiasConstantFactor = 0.0f;
    rasterizerCreateInto.depthBiasClamp = 0.0f;
    rasterizerCreateInto.depthBiasSlopeFactor = 0.0f;

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
    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
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

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
    colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> states = { VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = states.data();
    dynamicState.dynamicStateCount = states.size();

    VkPipelineLayout pipelineLayout;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    TK_ASSERT_VK_RESULT(
        vkCreatePipelineLayout(s_context->device, &pipelineLayoutInfo, s_context->allocationCallbacks, &pipelineLayout),
        "Could not create pipeline layout"
    );

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
            case ColorFormat::COLOR_FORMAT_R:
                colorFormats.emplace_back(VK_FORMAT_R8_SRGB);
                break;
            case ColorFormat::COLOR_FORMAT_RG:
                colorFormats.emplace_back(VK_FORMAT_R8G8_SRGB);
                break;
            case ColorFormat::COLOR_FORMAT_RGB:
                colorFormats.emplace_back(VK_FORMAT_R8G8B8_SRGB);
                break;
            case ColorFormat::COLOR_FORMAT_RGBA:
                colorFormats.emplace_back(VK_FORMAT_R8G8B8A8_SRGB);
                break;
            case ColorFormat::COLOR_FORMAT_DEPTH_STENCIL:
                renderingCreateInfo.depthAttachmentFormat = renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_D24_UNORM_S8_UINT;
                break;
            case ColorFormat::COLOR_FORMAT_DEPTH:
                renderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
                break;
            case ColorFormat::COLOR_FORMAT_STENCIL:
                renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_S8_UINT;
                break;
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
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;

    TK_ASSERT_VK_RESULT(
        vkCreateGraphicsPipelines(s_context->device, VK_NULL_HANDLE, 1, &pipelineInfo, s_context->allocationCallbacks, &m_pipeline),
        "Could not create graphics pipeline"
    );

    for (auto& shaderModule : shaderModules) {
        vkDestroyShaderModule(s_context->device, shaderModule, s_context->allocationCallbacks);
    }

    vkDestroyPipelineLayout(s_context->device, pipelineLayout, s_context->allocationCallbacks);
}

void VulkanGraphicsPipeline::destroy() {
    vkDestroyPipeline(s_context->device, m_pipeline, s_context->allocationCallbacks);
    vkDestroyPipelineLayout(s_context->device, m_pipelineLayout, s_context->allocationCallbacks);
}

static std::vector<uint32_t> getCompiledBinary(const std::pair<ShaderStage, std::filesystem::path>& stage) {
    shaderc::Compiler spirvCompiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);

#ifdef NDEBUG
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

    auto fileSource = TextLoader::readTextFile(stage.second);

    if (!fileSource.has_value()) {
        std::unreachable();
    }

    shaderc::SpvCompilationResult spirvModule =
        spirvCompiler.CompileGlslToSpv(fileSource.value().c_str(), shaderKind, std::filesystem::absolute(stage.second).string().c_str(), options);
    if (spirvModule.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
        std::println("ERROR MESSAGE:\n{}\n\nCOMPILATION STATUS: {}\n", spirvModule.GetErrorMessage(), (int) spirvModule.GetCompilationStatus());
    }
    TK_ASSERT(spirvModule.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success, "Shader compilation error");

    TK_ASSERT(spirvModule.end() - spirvModule.begin() > 0, "");
    return { spirvModule.begin(), spirvModule.end() };
}

}  // namespace Toki
