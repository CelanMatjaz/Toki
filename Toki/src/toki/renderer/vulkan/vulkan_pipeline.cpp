#include "vulkan_pipeline.h"

namespace Toki {


    vk::PipelineLayout VulkanPipeline::createPipelineLayout(
        const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
        const std::vector<vk::PushConstantRange>& pushConstants
    ) {
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.setSetLayouts(descriptorSetLayouts);
        pipelineLayoutCreateInfo.setPushConstantRanges(pushConstants);

        vk::PipelineLayout pipelineLayout;
        TK_ASSERT(VulkanRenderer::getDevice().createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout) == vk::Result::eSuccess);
        return pipelineLayout;
    }

    vk::Pipeline VulkanPipeline::createPipeline(const PipelineConfig& pipelineConfig) {
        const auto& [pipelineLayout, vertShaderModule, fragShaderModule, inputBindingDescriptions, inputAttributeDescriptions] = pipelineConfig;

        vk::Extent2D extent = VulkanRenderer::getSwapchainExtent();
        vk::RenderPass renderPass = VulkanRenderer::getRenderPass();

        vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo(
            {},
            vk::ShaderStageFlagBits::eVertex,
            vertShaderModule,
            "main"
        );

        vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo(
            {},
            vk::ShaderStageFlagBits::eFragment,
            fragShaderModule,
            "main"
        );

        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {
            vertShaderStageCreateInfo, fragShaderStageCreateInfo
        };

        vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
        vertexInputStateCreateInfo.setVertexBindingDescriptions(inputBindingDescriptions);
        vertexInputStateCreateInfo.setVertexAttributeDescriptions(inputAttributeDescriptions);

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
        inputAssemblyStateCreateInfo.topology = vk::PrimitiveTopology::eTriangleList; // REVIEW: maybe add to config?
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        vk::Viewport pipelineViewport;
        pipelineViewport.x = 0.0f;
        pipelineViewport.y = 0.0f;
        pipelineViewport.width = (float) extent.width;
        pipelineViewport.height = (float) extent.height;
        pipelineViewport.minDepth = 0.0f;
        pipelineViewport.maxDepth = 1.0f;

        vk::Rect2D pipelineScissor;
        pipelineScissor.extent = extent;

        vk::PipelineViewportStateCreateInfo viewportState{};
        viewportState.viewportCount = 1;
        viewportState.pViewports = &pipelineViewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &pipelineScissor;

        vk::PipelineRasterizationStateCreateInfo rasterizerCreateInto;
        rasterizerCreateInto.polygonMode = vk::PolygonMode::eFill; // REVIEW: maybe add to config?
        rasterizerCreateInto.frontFace = vk::FrontFace::eCounterClockwise; // REVIEW: maybe add to config?
        rasterizerCreateInto.cullMode = vk::CullModeFlagBits::eFront; // REVIEW: maybe add to config?
        rasterizerCreateInto.depthClampEnable = VK_FALSE;
        rasterizerCreateInto.rasterizerDiscardEnable = VK_FALSE;
        rasterizerCreateInto.depthBiasEnable = VK_FALSE;
        rasterizerCreateInto.depthBiasConstantFactor = 0.0f;
        rasterizerCreateInto.depthBiasClamp = 0.0f;
        rasterizerCreateInto.depthBiasSlopeFactor = 0.0f;

        vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
        multisampleStateCreateInfo.minSampleShading = 1.0f;
        multisampleStateCreateInfo.pSampleMask = nullptr;
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

        vk::PipelineDepthStencilStateCreateInfo  depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthCompareOp = vk::CompareOp::eLess;
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.minDepthBounds = 0.0f;
        depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
        depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        {
            using vk::ColorComponentFlagBits::eR;
            using vk::ColorComponentFlagBits::eG;
            using vk::ColorComponentFlagBits::eB;
            using vk::ColorComponentFlagBits::eA;
            colorBlendAttachment.colorWriteMask = static_cast<vk::ColorComponentFlagBits>(static_cast<uint32_t>(eR) | static_cast<uint32_t>(eG) | static_cast<uint32_t>(eB) | static_cast<uint32_t>(eA));
        }

        vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = vk::LogicOp::eCopy;
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
        colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

        vk::DynamicState states[] = { vk::DynamicState::eLineWidth };

        vk::PipelineDynamicStateCreateInfo dynamicState;
        dynamicState.pDynamicStates = states;
        dynamicState.dynamicStateCount = 1;

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizerCreateInto;
        pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
        pipelineInfo.pColorBlendState = &colorBlendStateCreateInfo;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencilStateCreateInfo;

        vk::Pipeline pipeline;
        TK_ASSERT(VulkanRenderer::getDevice().createGraphicsPipelines({}, 1, &pipelineInfo, nullptr, &pipeline) == vk::Result::eSuccess);
        return pipeline;
    }

    vk::DescriptorSetLayout VulkanPipeline::createDescriptorSetLayout(const DescriptorSetLayoutData& bindings) {
        std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(bindings.size());

        for (uint32_t i = 0; i < bindings.size(); ++i) {
            vk::DescriptorSetLayoutBinding binding{};
            binding.binding = bindings[i].binding;
            binding.descriptorType = bindings[i].descriptorType;
            binding.descriptorCount = bindings[i].count;
            binding.stageFlags = bindings[i].stageFlags;

            layoutBindings.push_back(binding);
        }

        vk::DescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.setBindings(layoutBindings);

        vk::DescriptorSetLayout descriptorSetLayout;
        TK_ASSERT(VulkanRenderer::getDevice().createDescriptorSetLayout(&layoutInfo, nullptr, &descriptorSetLayout) == vk::Result::eSuccess);
        return descriptorSetLayout;
    }

    vk::DescriptorPool VulkanPipeline::createDescriptorPool(vk::DescriptorType type, uint32_t size) {
        vk::DescriptorPoolSize poolSize{type, size};
        vk::DescriptorPoolCreateInfo poolInfo({}, size, 1, &poolSize);

        vk::DescriptorPool descriptorPool;
        TK_ASSERT(VulkanRenderer::getDevice().createDescriptorPool(&poolInfo, nullptr, &descriptorPool) == vk::Result::eSuccess);
        return descriptorPool;
    }

    std::vector<vk::DescriptorSet> VulkanPipeline::createDescriptorSets(vk::DescriptorPool descriptorPool, std::vector<vk::DescriptorSetLayout> descriptorSetLayouts) {
        vk::DescriptorSetAllocateInfo allocInfo;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = descriptorSetLayouts.size();
        allocInfo.pSetLayouts = descriptorSetLayouts.data();

        std::vector<vk::DescriptorSet> descriptorSets(descriptorSetLayouts.size());
        TK_ASSERT(VulkanRenderer::getDevice().allocateDescriptorSets(&allocInfo, descriptorSets.data()) == vk::Result::eSuccess);
        return descriptorSets;
    }

}