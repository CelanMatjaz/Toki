#include "vulkan_graphics_pipeline.h"

#include "core/application.h"
#include "core/assert.h"
#include "iostream"
#include "resources/loaders/shader_loader.h"
#include "vector"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_jobs/create_shader_module_job.h"
#include "vulkan/vulkan_pipelines/vulkan_pipeline.h"
#include "vulkan/vulkan_utils.h"

namespace Toki {

GraphicsPipeline::GraphicsPipeline(VulkanContext* context, const VulkanGraphicsPipelineConfig& config) : VulkanPipeline(context), config(config) {
    create();
}

GraphicsPipeline::~GraphicsPipeline() {
    destroy();
}

void GraphicsPipeline::recreate() {
    destroy();
    create();
}

void GraphicsPipeline::create() {
    auto code = ShaderLoader::loadShader(config.shaderPath);

    std::vector<Ref<CreateShaderModuleJob>> createShaderModuleJobs;
    std::vector<std::future<void>> futures;
    std::vector<VkShaderStageFlagBits> shaderStageFlagBits;

    for (const auto& [stage, binary] : code) {
        createShaderModuleJobs.emplace_back(CreateShaderModuleJob::create(context, binary));
        futures.emplace_back(Application::get().queueJob(createShaderModuleJobs.back()));
        shaderStageFlagBits.emplace_back(mapShaderStage(stage));
    }

    for (std::future<void>& future : futures) {
        future.wait();
    }

    std::vector<VkShaderModule> shaderModules;

    for (auto& job : createShaderModuleJobs) {
        shaderModules.emplace_back(job->getShaderModule());
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos(shaderModules.size());

    for (uint32_t i = 0; i < shaderModules.size(); ++i) {
        shaderStageCreateInfos[i] = {};
        shaderStageCreateInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfos[i].stage = shaderStageFlagBits[i];
        shaderStageCreateInfos[i].module = shaderModules[i];
        shaderStageCreateInfos[i].pName = "main";
    }

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = config.bindingDescriptions.size();
    vertexInputStateCreateInfo.pVertexBindingDescriptions = config.bindingDescriptions.data();
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = config.attributeDescriptions.size();
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = config.attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = mapPrimitiveTopology(config.topology);
    inputAssemblyStateCreateInfo.primitiveRestartEnable = config.primitiveRestartEnable;

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
    rasterizerCreateInto.cullMode = mapCullMode(config.cullMode);
    rasterizerCreateInto.frontFace = mapFrontFace(config.frontFace);
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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    TK_ASSERT_VK_RESULT(
        vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, context->allocationCallbacks, &pipelineLayout),
        "Could not create pipeline layout"
    );

    std::vector<VkDynamicState> states = { VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = states.data();
    dynamicState.dynamicStateCount = states.size();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
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
    pipelineInfo.renderPass = config.renderPass;
    pipelineInfo.subpass = config.subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;

    TK_ASSERT_VK_RESULT(
        vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, context->allocationCallbacks, &pipeline),
        "Could not create graphics pipeline"
    );

    for (const auto& shaderModule : shaderModules) {
        vkDestroyShaderModule(context->device, shaderModule, context->allocationCallbacks);
    }

    vkDestroyPipelineLayout(context->device, pipelineLayout, context->allocationCallbacks);
}

void GraphicsPipeline::destroy() {
    vkDeviceWaitIdle(context->device);
    vkDestroyPipeline(context->device, pipeline, context->allocationCallbacks);
}

}  // namespace Toki
