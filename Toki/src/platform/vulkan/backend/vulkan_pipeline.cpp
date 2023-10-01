#include "tkpch.h"
#include "vulkan_pipeline.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/vulkan/backend/vulkan_utils.h"
#include "core/assert.h"
#include "core/engine.h"

namespace Toki {

    Ref<Pipeline> Pipeline::create(const PipelineConfig& config) {
        switch (config.type) {
            case ShaderType::Graphics:
                return createRef<GraphicsPipeline>(config);
        }

        return nullptr;
    }

    Pipeline::Pipeline(const PipelineConfig& config) : config(std::move(config)) {
        TK_ASSERT(config.type != ShaderType::None, "ShaderType::None not supported for Vulkan pipeline!");
    };

    GraphicsPipeline::GraphicsPipeline(const PipelineConfig& config) : Pipeline(config) {
        create();
    }

    GraphicsPipeline::~GraphicsPipeline() {
        destroy();
    }

    void GraphicsPipeline::create() {
        auto window = Engine::getWindow();
        VkExtent2D extent = { window->getWidth(), window->getHeight() };
        const PipelineProperties props = config.properties;

        VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
        vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageCreateInfo.module = config.moduleSpirv[ShaderStage::Vertex];
        vertShaderStageCreateInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
        fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageCreateInfo.module = config.moduleSpirv[ShaderStage::Fragment];
        fragShaderStageCreateInfo.pName = "main";

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
            vertShaderStageCreateInfo, fragShaderStageCreateInfo
        };

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = config.inputBindingDescriptions.data();
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = config.inputBindingDescriptions.size();
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = config.inputAttributeDescriptions.data();
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = config.inputAttributeDescriptions.size();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = VulkanUtils::mapTopology(props.topology);
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkViewport pipelineViewport{};
        pipelineViewport.x = 0.0f;
        pipelineViewport.y = 0.0f;
        pipelineViewport.width = (float) extent.width;
        pipelineViewport.height = (float) extent.height;
        pipelineViewport.minDepth = 0.0f;
        pipelineViewport.maxDepth = 1.0f;

        VkRect2D pipelineScissor{};
        pipelineScissor.extent = extent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &pipelineViewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &pipelineScissor;

        VkPipelineRasterizationStateCreateInfo rasterizerCreateInto{};
        rasterizerCreateInto.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerCreateInto.polygonMode = props.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
        rasterizerCreateInto.frontFace = VulkanUtils::mapFrontFace(props.frontFace);
        rasterizerCreateInto.cullMode = VulkanUtils::mapCullMode(props.cullMode);
        rasterizerCreateInto.depthClampEnable = VK_FALSE;
        rasterizerCreateInto.rasterizerDiscardEnable = VK_FALSE;
        rasterizerCreateInto.depthBiasEnable = VK_FALSE;
        rasterizerCreateInto.depthBiasConstantFactor = 0.0f;
        rasterizerCreateInto.depthBiasClamp = 0.0f;
        rasterizerCreateInto.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // REVIEW: maybe add to config?
        multisampleStateCreateInfo.minSampleShading = 1.0f;
        multisampleStateCreateInfo.pSampleMask = nullptr;
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.depthTestEnable = props.enableDepthTest;
        depthStencilStateCreateInfo.depthWriteEnable = props.enableDepthWrite;
        depthStencilStateCreateInfo.depthCompareOp = VulkanUtils::mapCompareOp(props.depthCompareOp);
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.minDepthBounds = 0.0f;
        depthStencilStateCreateInfo.maxDepthBounds = 1.0f;

        if (props.enableStencilTest) {
            depthStencilStateCreateInfo.stencilTestEnable = VK_TRUE;
            depthStencilStateCreateInfo.back = VulkanUtils::mapStencilOpState(props.back);
            depthStencilStateCreateInfo.front = VulkanUtils::mapStencilOpState(props.front);
        }

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(((VulkanRenderPass*) config.renderPass.get())->getColorAttachmentCount(props.subpass), colorBlendAttachment);

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateCreateInfo.attachmentCount = colorBlendAttachments.size();
        colorBlendStateCreateInfo.pAttachments = colorBlendAttachments.data();
        colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> states = { VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pDynamicStates = states.data();
        dynamicState.dynamicStateCount = states.size();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizerCreateInto;
        pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
        pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = config.layout;
        pipelineInfo.renderPass = ((VulkanRenderPass*) config.renderPass.get())->getHandle();
        pipelineInfo.subpass = props.subpass;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;

        TK_ASSERT_VK_RESULT(vkCreateGraphicsPipelines(VulkanRenderer::device(), nullptr, 1, &pipelineInfo, nullptr, &pipeline), "Could not create graphics pipeline");
    }

    void GraphicsPipeline::destroy() {
        vkDeviceWaitIdle(VulkanRenderer::device());
        vkDestroyPipeline(VulkanRenderer::device(), pipeline, nullptr);
    }

};