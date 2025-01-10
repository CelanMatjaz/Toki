#include "vulkan_pipeline.h"

#include <utility>

#include "core/defer.h"
#include "renderer/vulkan/vulkan_utils.h"
#include "resources/loaders/text.h"

namespace toki {

void VulkanGraphicsPipeline::create(Ref<RendererContext> ctx, const Config& config) {
    std::vector<VkShaderModule> shader_modules(config.shader_config.stages.size());
    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos(shader_modules.size());

    for (u32 i = 0; i < shader_modules.size(); i++) {
        VkPipelineShaderStageCreateInfo& shader_stage_create_info = shader_stage_create_infos[i];
        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.module = shader_modules[i] = create_shader_module(ctx, config.shader_config.stages[i]);
        shader_stage_create_info.pName = "main";

        switch (config.shader_config.stages[i].stage) {
            case ShaderStage::VERTEX:
                shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case ShaderStage::FRAGMENT:
                shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            default:
                std::unreachable();
        }
    }

    Defer defer([ctx, shader_modules]() {
        for (auto& sm : shader_modules) {
            vkDestroyShaderModule(ctx->device, sm, ctx->allocation_callbacks);
        }
    });

    std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions(config.shader_config.bindings.size());
    for (u32 i = 0; i < vertex_binding_descriptions.size(); i++) {
        VkVertexInputBindingDescription& vertex_binding_description = vertex_binding_descriptions[i];
        vertex_binding_description.binding = config.shader_config.bindings[i].binding;
        vertex_binding_description.stride = config.shader_config.bindings[i].stride;

        switch (config.shader_config.bindings[i].inputRate) {
            case VertexInputRate::VERTEX:
                vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                break;
            case VertexInputRate::INSTANCE:
                vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                break;
        }
    }

    std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions(config.shader_config.attributes.size());
    for (u32 i = 0; i < vertex_binding_descriptions.size(); i++) {
        VkVertexInputAttributeDescription& vertex_attribute_description = vertex_attribute_descriptions[i];
        vertex_attribute_description.binding = config.shader_config.attributes[i].binding;
        vertex_attribute_description.offset = config.shader_config.attributes[i].offset;
        vertex_attribute_description.location = config.shader_config.attributes[i].location;

        switch (config.shader_config.attributes[i].format) {
            case VertexFormat::FLOAT1:
                vertex_attribute_description.format = VK_FORMAT_R32_SFLOAT;
                break;
            case VertexFormat::FLOAT2:
                vertex_attribute_description.format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case VertexFormat::FLOAT3:
                vertex_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case VertexFormat::FLOAT4:
                vertex_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            default:
                std::unreachable();
        }
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount = vertex_binding_descriptions.size();
    vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
    vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

    switch (config.shader_config.options.primitive_topology) {
        case PrimitiveTopology::POINT_LIST:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case PrimitiveTopology::LINE_LIST:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case PrimitiveTopology::LINE_STRIP:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case PrimitiveTopology::TRIANGLE_LIST:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case PrimitiveTopology::TRIANGLE_STRIP:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case PrimitiveTopology::TRIANGLE_FAN:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            break;
        case PrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
            break;
        case PrimitiveTopology::PATCH_LIST:
            input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            break;
    }

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.depthClampEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.lineWidth = 1.0f;
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    rasterization_state_create_info.depthBiasClamp = 0.0f;
    rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

    switch (config.shader_config.options.front_face) {
        case FrontFace::Clockwise:
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
            break;
        case FrontFace::CounterClockwise:
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            break;
    }

    switch (config.shader_config.options.polygon_mode) {
        case PolygonMode::FILL:
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            break;
        case PolygonMode::LINE:
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_LINE;
            break;
        case PolygonMode::POINT:
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_POINT;
            break;
    }

    switch (config.shader_config.options.cull_mode) {
        case CullMode::NONE:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
            break;
        case CullMode::FRONT:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_BIT;
            break;
        case CullMode::BACK:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case CullMode::BOTH:
            rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
            break;
    }

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.sampleShadingEnable = VK_FALSE;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_create_info.minSampleShading = 1.0f;
    multisample_state_create_info.pSampleMask = nullptr;
    multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_create_info.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
    depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state_create_info.depthTestEnable = config.shader_config.options.depth_test_enable ? VK_TRUE : VK_FALSE;
    depth_stencil_state_create_info.depthWriteEnable = config.shader_config.options.depth_write_enable ? VK_TRUE : VK_FALSE;
    depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_create_info.minDepthBounds = 0.0f;
    depth_stencil_state_create_info.maxDepthBounds = 1.0f;
    depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
    depth_stencil_state_create_info.front = {};
    depth_stencil_state_create_info.back = {};

    switch (config.shader_config.options.depth_compare_op) {
        case CompareOp::NEVER:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_NEVER;
            break;
        case CompareOp::LESS:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
            break;
        case CompareOp::EQUAL:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_EQUAL;
            break;
        case CompareOp::LESS_OR_EQUAL:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            break;
        case CompareOp::GREATER:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_GREATER;
            break;
        case CompareOp::NOT_EQUAL:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
            break;
        case CompareOp::GREATER_OR_EQUAL:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
            break;
        case CompareOp::ALWAYS:
            depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_ALWAYS;
            break;
    }

    VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
    color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_state.blendEnable = VK_FALSE;  // TODO: enable blending
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states(config.framebuffer.get_color_attachments_rendering_infos().size(), color_blend_attachment_state);

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_create_info.attachmentCount = color_blend_attachment_states.size();
    color_blend_state_create_info.pAttachments = color_blend_attachment_states.data();
    color_blend_state_create_info.blendConstants[0] = 1.0f;
    color_blend_state_create_info.blendConstants[1] = 1.0f;
    color_blend_state_create_info.blendConstants[2] = 1.0f;
    color_blend_state_create_info.blendConstants[3] = 1.0f;

    static std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
    dynamic_state_create_info.pDynamicStates = dynamic_states.data();

    VkViewport viewport{};
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    VkPushConstantRange push_constant{};
    push_constant.size = sizeof(glm::mat4);
    push_constant.offset = 0;
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pSetLayouts = nullptr;
    pipeline_layout_create_info.pushConstantRangeCount = 1;
    pipeline_layout_create_info.pPushConstantRanges = &push_constant;

    VK_CHECK(vkCreatePipelineLayout(ctx->device, &pipeline_layout_create_info, ctx->allocation_callbacks, &m_layout), "Could not create pipeline layout");

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info_create_info = config.framebuffer.get_pipeline_rendering_create_info();

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.pNext = &pipeline_rendering_info_create_info;
    graphics_pipeline_create_info.stageCount = shader_stage_create_infos.size();
    graphics_pipeline_create_info.pStages = shader_stage_create_infos.data();
    graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    graphics_pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
    graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
    graphics_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.layout = m_layout;

    TK_LOG_INFO("Creating new graphics pipeline");
    VK_CHECK(vkCreateGraphicsPipelines(ctx->device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, ctx->allocation_callbacks, &m_handle), "Could not create graphics pipeline");
}

void VulkanGraphicsPipeline::destroy(Ref<RendererContext> ctx) {
    vkDestroyPipeline(ctx->device, m_handle, ctx->allocation_callbacks);
    m_handle = VK_NULL_HANDLE;
    vkDestroyPipelineLayout(ctx->device, m_layout, ctx->allocation_callbacks);
    m_layout = VK_NULL_HANDLE;
}

VkPipeline VulkanGraphicsPipeline::get_handle() const {
    return m_handle;
}

VkPipelineLayout VulkanGraphicsPipeline::get_layout() const {
    return m_layout;
}

VkShaderModule VulkanGraphicsPipeline::create_shader_module(Ref<RendererContext> ctx, configs::Shader shader) {
    std::string shader_source = read_text_file("assets/shaders" / shader.path);
    std::vector shader_binary = compile_shader(shader.stage, shader_source);
    return toki::create_shader_module(ctx, shader_binary);
}

}  // namespace toki
