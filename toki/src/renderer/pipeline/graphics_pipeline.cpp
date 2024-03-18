#include "graphics_pipeline.h"

#include "renderer/vulkan_utils.h"
#include "toki/core/assert.h"

namespace Toki {

GraphicsPipeline::GraphicsPipeline(const ShaderBinaries& binaries, const GraphicsShaderOptions& options) : m_options(options) {
    m_pipeline = create(binaries);
}

VkPipelineBindPoint GraphicsPipeline::getBindPoint() const {
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
}

std::vector<VkVertexInputBindingDescription> createBindingDescriptions(std::vector<Toki::VertexBindingDescription>& descriptions);
std::vector<VkVertexInputAttributeDescription> createAttributeDescriptions(std::vector<Toki::VertexAttributeDescription>& descriptions);
std::vector<VkFormat> getColorAttachmentFormats(VkPipelineRenderingCreateInfoKHR& info, std::vector<Attachment> attachments);
VkPrimitiveTopology mapPrimitiveTopology(PrimitiveTopology topology);
VkPolygonMode mapPolygonMode(PolygonMode polygonMode);
VkCullModeFlags mapCullMode(CullMode cullMode);
VkFrontFace mapFrontFace(FrontFace frontFace);
VkCompareOp mapCompareOp(CompareOp compareOp);

VkPipeline GraphicsPipeline::create(const ShaderBinaries& binaries) {
    createLayoutFromBinaries(binaries);

    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
    createShaderStageCreateInfos(binaries, shaderStageCreateInfos);

    std::vector<VkVertexInputBindingDescription> bindingDescriptions = createBindingDescriptions(m_options.layoutDescriptions.bindingDescriptions);
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
        createAttributeDescriptions(m_options.layoutDescriptions.attributeDescriptions);

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = m_options.primitiveRestart ? VK_TRUE : VK_FALSE;
    inputAssemblyStateCreateInfo.topology = mapPrimitiveTopology(m_options.primitiveTopology);

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
    rasterizationStateCreateInfo.frontFace = mapFrontFace(m_options.frontFace);
    rasterizationStateCreateInfo.polygonMode = mapPolygonMode(m_options.polygonMode);
    rasterizationStateCreateInfo.cullMode = mapCullMode(m_options.cullMode);

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
    depthStencilStateCreateInfo.depthTestEnable = m_options.depthTest.enable ? VK_TRUE : VK_FALSE;
    depthStencilStateCreateInfo.depthWriteEnable = m_options.depthTest.write ? VK_TRUE : VK_FALSE;
    depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.minDepthBounds = 0.0f;
    depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
    depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.front = {};
    depthStencilStateCreateInfo.back = {};

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    colorBlendAttachmentState.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.blendEnable = VK_TRUE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    uint32_t colorAttachmentCount = 0;
    for (const auto& attachment : m_options.attachments) {
        if (attachment.colorFormat == ColorFormat::COLOR_FORMAT_RGBA) {
            ++colorAttachmentCount;
        }
    }

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates(colorAttachmentCount, colorBlendAttachmentState);

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.attachmentCount = colorBlendAttachmentStates.size();
    colorBlendStateCreateInfo.pAttachments = colorBlendAttachmentStates.data();
    colorBlendStateCreateInfo.blendConstants[0] = 1.0f;
    colorBlendStateCreateInfo.blendConstants[1] = 1.0f;
    colorBlendStateCreateInfo.blendConstants[2] = 1.0f;
    colorBlendStateCreateInfo.blendConstants[3] = 1.0f;

    std::vector<VkDynamicState> states = { VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = states.data();
    dynamicState.dynamicStateCount = states.size();

    VkPipelineRenderingCreateInfoKHR renderingCreateInfo{};
    renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.viewMask = 0;
    std::vector<VkFormat> colorFormats = getColorAttachmentFormats(renderingCreateInfo, m_options.attachments);
    renderingCreateInfo.colorAttachmentCount = colorFormats.size();
    renderingCreateInfo.pColorAttachmentFormats = colorFormats.data();

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &renderingCreateInfo;
    pipelineInfo.stageCount = shaderStageCreateInfos.size();
    pipelineInfo.pStages = shaderStageCreateInfos.data();
    pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    pipelineInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
    pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    pipelineInfo.pViewportState = &viewportState;

    VkPipeline pipeline = VK_NULL_HANDLE;

    TK_ASSERT_VK_RESULT(
        vkCreateGraphicsPipelines(s_context->device, VK_NULL_HANDLE, 1, &pipelineInfo, s_context->allocationCallbacks, &pipeline),
        "Could not create graphics pipeline");

    for (VkPipelineShaderStageCreateInfo& shaderStageCreateInfo : shaderStageCreateInfos) {
        vkDestroyShaderModule(s_context->device, shaderStageCreateInfo.module, s_context->allocationCallbacks);
    }

    return pipeline;
}

std::vector<VkVertexInputBindingDescription> createBindingDescriptions(std::vector<Toki::VertexBindingDescription>& descriptions) {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(descriptions.size());

    for (uint32_t i = 0; i < descriptions.size(); ++i) {
        bindingDescriptions[i].stride = descriptions[i].stride;
        bindingDescriptions[i].binding = descriptions[i].binding;

        switch (descriptions[i].inputRate) {
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

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> createAttributeDescriptions(std::vector<Toki::VertexAttributeDescription>& descriptions) {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(descriptions.size());

    for (uint32_t i = 0; i < descriptions.size(); ++i) {
        attributeDescriptions[i].binding = descriptions[i].binding;
        attributeDescriptions[i].offset = descriptions[i].offset;
        attributeDescriptions[i].location = descriptions[i].location;

        switch (descriptions[i].format) {
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

    return attributeDescriptions;
}

std::vector<VkFormat> getColorAttachmentFormats(VkPipelineRenderingCreateInfoKHR& info, std::vector<Attachment> attachments) {
    std::vector<VkFormat> colorFormats;

    for (const auto& attachment : attachments) {
        if (attachment.presentable) {
            colorFormats.emplace_back(VK_FORMAT_B8G8R8A8_SRGB);
            continue;
        }

        switch (attachment.colorFormat) {
            case ColorFormat::COLOR_FORMAT_DEPTH:
                info.depthAttachmentFormat = VulkanUtils::mapFormat(attachment.colorFormat);
                break;

            case ColorFormat::COLOR_FORMAT_STENCIL:
                info.stencilAttachmentFormat = VulkanUtils::mapFormat(attachment.colorFormat);
                break;

            case ColorFormat::COLOR_FORMAT_DEPTH_STENCIL:
                info.depthAttachmentFormat = info.stencilAttachmentFormat = VulkanUtils::mapFormat(attachment.colorFormat);
                break;

            default:
                std::unreachable();
        }
    }

    return colorFormats;
}

VkPrimitiveTopology mapPrimitiveTopology(PrimitiveTopology topology) {
    switch (topology) {
        case PrimitiveTopology::PointList:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        case PrimitiveTopology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

        case PrimitiveTopology::LineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

        case PrimitiveTopology::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        case PrimitiveTopology::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

        case PrimitiveTopology::TriangleFan:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

        case PrimitiveTopology::LineListWithAdjacency:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;

        case PrimitiveTopology::LineStripWithAdjacency:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;

        case PrimitiveTopology::TriangleListWithAdjacency:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;

        case PrimitiveTopology::TriangleStripWithAdjacency:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;

        case PrimitiveTopology::PatchList:
            return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

        default:
            std::unreachable();
    }
}

VkPolygonMode mapPolygonMode(PolygonMode polygonMode) {
    switch (polygonMode) {
        case PolygonMode::Fill:
            return VK_POLYGON_MODE_FILL;

        case PolygonMode::Line:
            return VK_POLYGON_MODE_LINE;

        case PolygonMode::Point:
            return VK_POLYGON_MODE_POINT;

        default:
            std::unreachable();
    }
}

VkCullModeFlags mapCullMode(CullMode cullMode) {
    switch (cullMode) {
        case CullMode::None:
            return VK_CULL_MODE_NONE;

        case CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;

        case CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;

        case CullMode::FrontAndBack:
            return VK_CULL_MODE_FRONT_AND_BACK;

        default:
            std::unreachable();
    }
}

VkFrontFace mapFrontFace(FrontFace frontFace) {
    switch (frontFace) {
        case FrontFace::CounterClockwise:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;

        case FrontFace::Clockwise:
            return VK_FRONT_FACE_CLOCKWISE;

        default:
            std::unreachable();
    }
}

VkCompareOp mapCompareOp(CompareOp compareOp) {
    switch (compareOp) {
        case CompareOp::Always:
            return VK_COMPARE_OP_ALWAYS;

        case CompareOp::Equal:
            return VK_COMPARE_OP_EQUAL;

        case CompareOp::Greater:
            return VK_COMPARE_OP_GREATER;

        case CompareOp::GreaterOrEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;

        case CompareOp::Less:
            return VK_COMPARE_OP_LESS;

        case CompareOp::LessOrEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;

        case CompareOp::Never:
            return VK_COMPARE_OP_NEVER;

        case CompareOp::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;

        default:
            std::unreachable();
    }
}

}  // namespace Toki
