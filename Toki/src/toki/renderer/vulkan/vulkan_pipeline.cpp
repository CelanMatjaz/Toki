#include "vulkan_pipeline.h"

namespace Toki {

    uint32_t VulkanPipeline::createPipelineLayout(
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const std::vector<VkPushConstantRange>& pushConstants
    ) {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();
        pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstants.size();

        VkPipelineLayout pipelineLayout;
        TK_ASSERT(vkCreatePipelineLayout(VulkanRenderer::getDevice(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) == VK_SUCCESS);
        pipelineLayouts.emplace_back(pipelineLayout);
        return pipelineLayouts.size() - 1;
    }

    uint32_t VulkanPipeline::createPipeline(const PipelineConfig& pipelineConfig) {
        PipelineData data = { _createPipeline(pipelineConfig), pipelineConfig };
        pipelines.emplace_back(data);
        return pipelines.size() - 1;
    }

    VkPipeline VulkanPipeline::_createPipeline(const PipelineConfig& pipelineConfig) {
        const auto& [pipelineLayoutIndex, vertShaderIndex, fragShaderIndex, inputBindingDescriptions, inputAttributeDescriptions] = pipelineConfig;

        VkExtent2D extent = VulkanRenderer::getSwapchainExtent();
        VkRenderPass renderPass = VulkanRenderer::getRenderPass();

        VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
        vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageCreateInfo.module = shaderModules[vertShaderIndex];
        vertShaderStageCreateInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
        fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageCreateInfo.module = shaderModules[fragShaderIndex];
        fragShaderStageCreateInfo.pName = "main";

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
            vertShaderStageCreateInfo, fragShaderStageCreateInfo
        };

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = inputBindingDescriptions.data();
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = inputBindingDescriptions.size();
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = inputAttributeDescriptions.data();
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = inputAttributeDescriptions.size();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // REVIEW: maybe add to config?
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
        rasterizerCreateInto.polygonMode = VK_POLYGON_MODE_FILL; // REVIEW: maybe add to config?
        rasterizerCreateInto.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // REVIEW: maybe add to config?
        rasterizerCreateInto.cullMode = VK_CULL_MODE_BACK_BIT; // REVIEW: maybe add to config?
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
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // REVIEW: maybe add to config?
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.minDepthBounds = 0.0f;
        depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
        depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

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

        VkDynamicState states[] = { VK_DYNAMIC_STATE_LINE_WIDTH };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pDynamicStates = states;
        dynamicState.dynamicStateCount = 1;

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
        pipelineInfo.layout = pipelineLayouts[pipelineLayoutIndex];
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;

        VkPipeline pipeline;
        TK_ASSERT(vkCreateGraphicsPipelines(VulkanRenderer::getDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline) == VK_SUCCESS);
        return pipeline;
    }

    VkDescriptorSetLayout VulkanPipeline::createDescriptorSetLayout(const DescriptorSetLayoutData& bindings) {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(bindings.size());

        for (uint32_t i = 0; i < bindings.size(); ++i) {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = bindings[i].binding;
            binding.descriptorType = bindings[i].descriptorType;
            binding.descriptorCount = bindings[i].count;
            binding.stageFlags = bindings[i].stageFlags;

            layoutBindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pBindings = layoutBindings.data();
        layoutInfo.bindingCount = layoutBindings.size();

        VkDescriptorSetLayout descriptorSetLayout;
        TK_ASSERT(vkCreateDescriptorSetLayout(VulkanRenderer::getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) == VK_SUCCESS);
        return descriptorSetLayout;
    }

    VkDescriptorPool VulkanPipeline::createDescriptorPool(const DestriptorPoolSizes& sizes) {
        uint32_t maxSets = std::accumulate(sizes.begin(), sizes.end(), 0, [](int val, const DestriptorPoolSize& size) {
            return val + size.size;
        });

        std::vector<VkDescriptorPoolSize> poolSizes(sizes.size());

        for (uint32_t i = 0; i < sizes.size(); ++i) {
            poolSizes[i].type = sizes[i].type;
            poolSizes[i].descriptorCount = sizes[i].size;
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.maxSets = maxSets;

        VkDescriptorPool descriptorPool;
        TK_ASSERT(vkCreateDescriptorPool(VulkanRenderer::getDevice(), &poolInfo, nullptr, &descriptorPool) == VK_SUCCESS);
        return descriptorPool;
    }

    std::vector<VkDescriptorSet> VulkanPipeline::createDescriptorSets(VkDescriptorPool descriptorPool, std::vector<VkDescriptorSetLayout> descriptorSetLayouts) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = descriptorSetLayouts.size();
        allocInfo.pSetLayouts = descriptorSetLayouts.data();

        std::vector<VkDescriptorSet> descriptorSets(descriptorSetLayouts.size());
        TK_ASSERT(vkAllocateDescriptorSets(VulkanRenderer::getDevice(), &allocInfo, descriptorSets.data()) == VK_SUCCESS);
        return descriptorSets;
    }

    std::vector<char> VulkanPipeline::loadShaderCode(const std::filesystem::path& filePath) {
        std::ifstream file(filePath.string(), std::ios::ate | std::ios::binary);
        uint32_t fileSize = file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    uint32_t VulkanPipeline::createShaderModule(const std::filesystem::path& filePath) {
        auto data = loadShaderCode(filePath);

        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = data.size();
        shaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(data.data());

        VkShaderModule shaderModule;
        TK_ASSERT(vkCreateShaderModule(VulkanRenderer::getDevice(), &shaderModuleCreateInfo, nullptr, &shaderModule) == VK_SUCCESS);
        shaderModules.emplace_back(std::move(shaderModule));
        return shaderModules.size() - 1;
    }

    void VulkanPipeline::recreatePipelines() {
        VkDevice device = VulkanRenderer::getDevice();

        vkDeviceWaitIdle(device);
        for (uint32_t i = 0; i < pipelines.size(); ++i) {
            vkDestroyPipeline(device, pipelines[i].pipeline, nullptr);
            pipelines[i].pipeline = VulkanPipeline::_createPipeline(pipelines[i].config);
        }
    }

    const VkPipeline VulkanPipeline::getPipeline(uint32_t index) {
        if (index < pipelines.size())
            return pipelines[index].pipeline;

        throw std::runtime_error("Pipeline with specified index does not exist");
    }

    const VkPipelineLayout VulkanPipeline::getPipelineLayout(uint32_t index) {
        if (index < pipelineLayouts.size())
            return pipelineLayouts[index];

        throw std::runtime_error("Pipeline layout with specified index does not exist");
    }

    void VulkanPipeline::cleanup() {
        VkDevice device = VulkanRenderer::getDevice();
        for (const auto& [pipeline, config] : pipelines) vkDestroyPipeline(device, pipeline, nullptr);
        for (const auto& shaderModule : shaderModules) vkDestroyShaderModule(device, shaderModule, nullptr);
        for (const auto& pipelineLayout : pipelineLayouts) vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
}