#include "pipeline.h"

#include <spirv_cross/spirv_cross.hpp>
#include <unordered_map>

#include "renderer/mapping_functions.h"
#include "renderer/renderer_state.h"
#include "renderer/vulkan_types.h"
#include "toki/core/assert.h"
#include "toki/renderer/renderer_types.h"
#include "toki/resources/loaders/text_loader.h"
#include "toki/resources/resource_types.h"
#include "toki/resources/resource_utils.h"
#include "toki/resources/serializers/shader_serializer.h"
#include "toki/resources/shader_compiler.h"

namespace Toki {

static void reflectConstants(ShaderStage stage, spirv_cross::Compiler& compiler, std::vector<VkPushConstantRange>& pushConstants) {
    for (const auto& constant : compiler.get_shader_resources().push_constant_buffers) {
        auto elementType = compiler.get_type(constant.base_type_id);

        VkPushConstantRange range{};
        range.size = compiler.get_declared_struct_size(elementType);
        range.offset = compiler.get_decoration(constant.id, spv::DecorationOffset);
        range.stageFlags = mapShaderStage(stage);

        auto result = std::find_if(
            pushConstants.begin(), pushConstants.end(), [r = range](VkPushConstantRange constant) { return r.offset == constant.offset; });

        if (result != pushConstants.end()) {
            result->stageFlags |= range.stageFlags;
            continue;
        }

        pushConstants.emplace_back(range);
    }
}

static void reflectDescriptors(ShaderStage stage, spirv_cross::Compiler& compiler, SetBindings& setBindings) {
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
            // const auto& type = compiler.get_type(resource.base_type_id);
            const auto& typeArray = compiler.get_type(resource.type_id);

            uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

            VkDescriptorSetLayoutBinding binding{};
            binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            binding.descriptorCount = typeArray.array.size() == 0 ? 1 : typeArray.array[0];
            binding.descriptorType = descriptorType;
            binding.stageFlags = mapShaderStage(stage);

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

static ShaderPipelineBinaries getBinaries(const ShaderConfig& shaderConfig) {
    ShaderPipelineBinaries stageBinaries;

    for (const auto& [stage, sourcePath, binaryPath] : shaderConfig.stages) {
        if (ResourceUtils::fileExists(binaryPath)) {
            stageBinaries.emplace(stage, std::move(*ShaderSerializer::loadShaderBinary(binaryPath)));
        } else if (ResourceUtils::fileExists(sourcePath)) {
            auto shaderSourceReadResult = TextLoader::readTextFile(sourcePath);
            TK_ASSERT(shaderSourceReadResult.has_value(), "Shader source file '{}' was not parsed correctly", sourcePath.string());

            auto compileResult = ShaderCompiler::compileShader(shaderSourceReadResult.value(), stage);
            TK_ASSERT(compileResult.has_value(), "Shader source file '{}' compilation error", sourcePath.string());

            stageBinaries.emplace(stage, std::move(compileResult.value()));
            ShaderSerializer::serialize(binaryPath.filename(), stageBinaries[stage]);
        } else {
            TK_ASSERT(
                false,
                "Shader source and/or binary path(s) do not exist or are invalid (source: '',binary: '')",
                sourcePath.string(),
                binaryPath.string());
        }
    }

    return stageBinaries;
}

void Pipeline::allocateDescriptorSets(std::vector<VkDescriptorSetLayout>& layouts) {
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = s_descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = layouts.size();
    descriptorSetAllocateInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(layouts.size());

    if (m_descriptorSets.size() == 0) {
        return;
    }

    TK_ASSERT_VK_RESULT(
        vkAllocateDescriptorSets(context.device, &descriptorSetAllocateInfo, m_descriptorSets.data()), "Could not allocate descriptor sets");
}

void Pipeline::createPipelineLayout(const ShaderPipelineBinaries& binaries) {
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
            vkCreateDescriptorSetLayout(context.device, &descriptorSetLayoutCreateInfo, context.allocationCallbacks, &m_descriptorSetLayouts[set]),
            "Could not create descriptor set layout");
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRanges.size();
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
    pipelineLayoutCreateInfo.setLayoutCount = m_descriptorSetLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = m_descriptorSetLayouts.data();

    TK_ASSERT_VK_RESULT(
        vkCreatePipelineLayout(context.device, &pipelineLayoutCreateInfo, context.allocationCallbacks, &m_pipelineLayout),
        "Could not create pipeline layout");

    m_descriptorSets.resize(m_descriptorSetLayouts.size());

    if (m_descriptorSets.size() > 0) {
        allocateDescriptorSets(m_descriptorSetLayouts);
    }
}

Pipeline::Pipeline(const ShaderConfig& config) : m_renderPassHandle(config.attachments), m_config(config) {
    if (!s_renderPassMap.contains(m_renderPassHandle)) {
        s_renderPassMap.emplace(m_renderPassHandle, createRef<RenderPass>(config.attachments));
    }

    auto binaries = getBinaries(m_config);

    createPipelineLayout(binaries);
    create(binaries);
}

Pipeline::~Pipeline() {
    destroy();
}

static std::vector<VkVertexInputBindingDescription> createBindingDescriptions(std::vector<Binding>& bindings);
static std::vector<VkVertexInputAttributeDescription> createAttributeDescriptions(std::vector<Attribute>& attribute);
static std::vector<VkFormat> getColorAttachmentFormats(VkPipelineRenderingCreateInfoKHR& info, std::vector<Attachment> attachments);
static VkPrimitiveTopology mapPrimitiveTopology(PrimitiveTopology topology);
static VkPolygonMode mapPolygonMode(PolygonMode polygonMode);
static VkCullModeFlags mapCullMode(CullMode cullMode);
static VkFrontFace mapFrontFace(FrontFace frontFace);
static VkCompareOp mapCompareOp(CompareOp compareOp);

static void createShaderStageCreateInfos(
    const ShaderPipelineBinaries& binaries, std::vector<VkPipelineShaderStageCreateInfo>& shaderStageCreateInfos) {
    for (auto& [stage, binary] : binaries) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = binary.size() * 4;
        shaderModuleCreateInfo.pCode = binary.data();

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.pName = "main";
        shaderStageCreateInfo.stage = mapShaderStage(stage);

        TK_ASSERT_VK_RESULT(
            vkCreateShaderModule(context.device, &shaderModuleCreateInfo, context.allocationCallbacks, &shaderStageCreateInfo.module),
            "Could not create shader module");

        shaderStageCreateInfos.emplace_back(shaderStageCreateInfo);
    }
}

void Pipeline::create(const ShaderPipelineBinaries& binaries) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
    createShaderStageCreateInfos(binaries, shaderStageCreateInfos);

    std::vector<VkVertexInputBindingDescription> bindingDescriptions = createBindingDescriptions(m_config.bindings);
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = createAttributeDescriptions(m_config.attributes);

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = m_config.options.primitiveRestart ? VK_TRUE : VK_FALSE;
    inputAssemblyStateCreateInfo.topology = mapPrimitiveTopology(m_config.options.primitiveTopology);

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
    rasterizationStateCreateInfo.frontFace = mapFrontFace(m_config.options.frontFace);
    rasterizationStateCreateInfo.polygonMode = mapPolygonMode(m_config.options.polygonMode);
    rasterizationStateCreateInfo.cullMode = mapCullMode(m_config.options.cullMode);

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

    for (const auto& a : m_config.attachments) {
        if (a.colorFormat == ColorFormat::R8 || a.colorFormat == ColorFormat::RG8 || a.colorFormat == ColorFormat::RGBA8) {
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

    static std::vector<VkDynamicState> states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = states.size();
    dynamicStateCreateInfo.pDynamicStates = states.data();

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = shaderStageCreateInfos.size();
    pipelineCreateInfo.pStages = shaderStageCreateInfos.data();
    pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineCreateInfo.renderPass = *s_renderPassMap[m_renderPassHandle];
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.layout = m_pipelineLayout;

    TK_ASSERT_VK_RESULT(
        vkCreateGraphicsPipelines(context.device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_pipeline), "Could not create graphics pipeline");

    // if (parent != VK_NULL_HANDLE) {
    // pipelineCreateInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    //     pipelineCreateInfo.basePipelineHandle = parent;
    //     pipelineCreateInfo.basePipelineIndex = -1;
    // }

    for (auto& stage : shaderStageCreateInfos) {
        vkDestroyShaderModule(context.device, stage.module, context.allocationCallbacks);
    }
}

void Pipeline::destroy() {
    for (auto& layout : m_descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(context.device, layout, context.allocationCallbacks);
    }

    vkDestroyPipeline(context.device, m_pipeline, context.allocationCallbacks);

    for (auto& [_, childPipeline] : m_childPipelines) {
        childPipeline.destroy();
    }

    m_childPipelines.clear();

    if (!m_hasParent) {
        vkDestroyPipelineLayout(context.device, m_pipelineLayout, context.allocationCallbacks);
    }
}

static std::vector<VkVertexInputBindingDescription> createBindingDescriptions(std::vector<Binding>& bindings) {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(bindings.size());

    for (uint32_t i = 0; i < bindings.size(); ++i) {
        bindingDescriptions[i].stride = bindings[i].stride;
        bindingDescriptions[i].binding = bindings[i].binding;

        switch (bindings[i].type) {
            case VertexInputRate::Vertex:
                bindingDescriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                break;

            case VertexInputRate::Instance:
                bindingDescriptions[i].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                break;

            default:
                std::unreachable();
        }
    }

    return bindingDescriptions;
}

static std::vector<VkVertexInputAttributeDescription> createAttributeDescriptions(std::vector<Attribute>& attributes) {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(attributes.size());

    uint32_t offset = 0;

    for (uint32_t i = 0; i < attributes.size(); ++i) {
        attributeDescriptions[i].binding = attributes[i].binding;
        attributeDescriptions[i].offset = offset;
        attributeDescriptions[i].location = attributes[i].location;

        offset += attributes[i].getTypeSize();

        switch (attributes[i].type) {
            case AttributeType::vec1:
                attributeDescriptions[i].format = VK_FORMAT_R32_SFLOAT;
                break;

            case AttributeType::vec2:
                attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
                break;

            case AttributeType::vec3:
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                break;

            case AttributeType::vec4:
                attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;

            default:
                std::unreachable();
        }
    }

    return attributeDescriptions;
}

static std::vector<VkFormat> getColorAttachmentFormats(VkPipelineRenderingCreateInfoKHR& info, std::vector<Attachment> attachments) {
    std::vector<VkFormat> colorFormats;

    for (const auto& attachment : attachments) {
        if (attachment.presentable) {
            colorFormats.emplace_back(VK_FORMAT_B8G8R8A8_SRGB);
            continue;
        }

        switch (attachment.colorFormat) {
            case ColorFormat::Depth:
                info.depthAttachmentFormat = mapFormat(attachment.colorFormat);
                break;

            case ColorFormat::Stencil:
                info.stencilAttachmentFormat = mapFormat(attachment.colorFormat);
                break;

            case ColorFormat::DepthStencil:
                info.depthAttachmentFormat = info.stencilAttachmentFormat = mapFormat(attachment.colorFormat);
                break;

            default:
                std::unreachable();
        }
    }

    return colorFormats;
}

static VkPrimitiveTopology mapPrimitiveTopology(PrimitiveTopology topology) {
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

static VkPolygonMode mapPolygonMode(PolygonMode polygonMode) {
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

static VkCullModeFlags mapCullMode(CullMode cullMode) {
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
