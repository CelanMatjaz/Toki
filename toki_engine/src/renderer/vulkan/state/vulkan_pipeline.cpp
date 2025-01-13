#include "vulkan_pipeline.h"

#include <utility>

#include "core/assert.h"
#include "core/defer.h"
#include "renderer/vulkan/vulkan_types.h"
#include "renderer/vulkan/vulkan_utils.h"
#include "resources/loaders/text.h"
#include "vulkan/vulkan_core.h"

namespace toki {

void VulkanGraphicsPipeline::create(Ref<RendererContext> ctx, const Config& config) {
    std::vector<VkShaderModule> shader_modules(config.shader_config.stages.size());
    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos(shader_modules.size());

    PipelineResources resources = create_pipeline_resources(ctx, config.shader_config.stages);
    m_layout = resources.layout;

    for (u32 i = 0; const auto& [stage, module] : resources.shader_modules) {
        VkPipelineShaderStageCreateInfo& shader_stage_create_info = shader_stage_create_infos[i++];
        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.module = module;
        shader_stage_create_info.pName = "main";
        shader_stage_create_info.stage = map_shader_stage(stage);
    }

    Defer defer([ctx, shader_stage_create_infos]() {
        for (auto& sm : shader_stage_create_infos) {
            vkDestroyShaderModule(ctx->device, sm.module, ctx->allocation_callbacks);
        }
    });

    std::vector<VkVertexInputBindingDescription> vertex_binding_descriptions(config.shader_config.bindings.size());
    for (u32 i = 0; i < vertex_binding_descriptions.size(); i++) {
        vertex_binding_descriptions[i].binding = config.shader_config.bindings[i].binding;
        vertex_binding_descriptions[i].stride = config.shader_config.bindings[i].stride;

        switch (config.shader_config.bindings[i].inputRate) {
            case VertexInputRate::VERTEX:
                vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                break;
            case VertexInputRate::INSTANCE:
                vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
                break;
        }
    }

    std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions(config.shader_config.attributes.size());
    for (u32 i = 0; i < vertex_attribute_descriptions.size(); i++) {
        vertex_attribute_descriptions[i].binding = config.shader_config.attributes[i].binding;
        vertex_attribute_descriptions[i].offset = config.shader_config.attributes[i].offset;
        vertex_attribute_descriptions[i].location = config.shader_config.attributes[i].location;

        switch (config.shader_config.attributes[i].format) {
            case VertexFormat::FLOAT1:
                vertex_attribute_descriptions[i].format = VK_FORMAT_R32_SFLOAT;
                break;
            case VertexFormat::FLOAT2:
                vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case VertexFormat::FLOAT3:
                vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case VertexFormat::FLOAT4:
                vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
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
    color_blend_attachment_state.blendEnable = VK_TRUE;  // TODO: enable blending
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states(config.framebuffer.get_color_formats().size(), color_blend_attachment_state);

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

    const auto& color_formats = config.framebuffer.get_color_formats();

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info_create_info{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    pipeline_rendering_info_create_info.colorAttachmentCount = color_formats.size();
    pipeline_rendering_info_create_info.pColorAttachmentFormats = color_formats.data();
    pipeline_rendering_info_create_info.depthAttachmentFormat = config.framebuffer.get_depth_format();
    pipeline_rendering_info_create_info.stencilAttachmentFormat = config.framebuffer.get_stencil_format();

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
    graphics_pipeline_create_info.layout = m_layout.handle;

    TK_LOG_INFO("Creating new graphics pipeline");
    VK_CHECK(vkCreateGraphicsPipelines(ctx->device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, ctx->allocation_callbacks, &m_handle), "Could not create graphics pipeline");
}

void VulkanGraphicsPipeline::destroy(Ref<RendererContext> ctx) {
    vkDestroyPipeline(ctx->device, m_handle, ctx->allocation_callbacks);
    m_handle = VK_NULL_HANDLE;

    for (auto& layout : m_layout.descriptor_set_layouts) {
        vkDestroyDescriptorSetLayout(ctx->device, layout, ctx->allocation_callbacks);
    }

    vkDestroyPipelineLayout(ctx->device, m_layout.handle, ctx->allocation_callbacks);
    m_layout.handle = {};
}

VkPipeline VulkanGraphicsPipeline::get_handle() const {
    return m_handle;
}

VkPipelineLayout VulkanGraphicsPipeline::get_layout() const {
    return m_layout.handle;
}

VkShaderStageFlags VulkanGraphicsPipeline::get_push_constant_stage_bits() const {
    return m_layout.push_constant_stage_bits;
}

const std::vector<VkDescriptorSet>& VulkanGraphicsPipeline::get_descriptor_sets() const {
    return m_descriptorSets;
}

void VulkanGraphicsPipeline::allocate_descriptor_sets(Ref<RendererContext> ctx, DescriptorPoolManager& pool_manager) {
    m_descriptorSets = pool_manager.allocate_multiple(ctx, m_layout.descriptor_set_layouts);
}

PipelineLayout VulkanGraphicsPipeline::create_pipeline_layout(Ref<RendererContext> ctx, const PipelineLayoutCreateConfig& config) {
    PipelineLayout layout;
    layout.descriptor_set_layouts.reserve(MAX_DESCRIPTOR_SETS);

    for (u32 i = 0; i < config.bindings.binding_counts.size(); i++) {
        u32 binding_count = config.bindings.binding_counts[i];
        if (binding_count == 0) {
            continue;
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.bindingCount = binding_count;
        descriptor_set_layout_create_info.pBindings = &config.bindings.bindings[MAX_DESCRIPTOR_SETS * i];

        VkDescriptorSetLayout descriptor_set_layout;
        VK_CHECK(
            vkCreateDescriptorSetLayout(ctx->device, &descriptor_set_layout_create_info, ctx->allocation_callbacks, &descriptor_set_layout) != VK_SUCCESS, "Could not create descriptor set layout");
        layout.descriptor_set_layouts.emplace_back(descriptor_set_layout);
    }

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = layout.descriptor_set_layouts.size();
    pipeline_layout_create_info.pSetLayouts = layout.descriptor_set_layouts.data();
    pipeline_layout_create_info.pushConstantRangeCount = std::min(config.push_constants.size(), (decltype(config.push_constants.size())) 1);
    pipeline_layout_create_info.pPushConstantRanges = config.push_constants.data();

    VK_CHECK(vkCreatePipelineLayout(ctx->device, &pipeline_layout_create_info, ctx->allocation_callbacks, &layout.handle), "Could not create pipeline layout");

    if (config.push_constants.size() > 0) {
        layout.push_constant_stage_bits = config.push_constants[0].stageFlags;
    }

    return layout;
}

void VulkanGraphicsPipeline::reflect_shader(ShaderStage stage, std::vector<u32>& binary, DescriptorBindings& bindings, std::vector<VkPushConstantRange>& push_constants) {
    spirv_cross::Compiler compiler(binary);
    const auto& resources = compiler.get_shader_resources();

    VkShaderStageFlags shader_stage = map_shader_stage(stage);

    for (u32 i = 0; i < resources.push_constant_buffers.size(); i++) {
        auto element_type = compiler.get_type(resources.push_constant_buffers[i].base_type_id);
        if (push_constants.size() == 1) {
            push_constants[i].stageFlags |= shader_stage;
        } else {
            VkPushConstantRange push_constant;
            push_constant.size = compiler.get_declared_struct_size(element_type);
            push_constant.offset = compiler.get_decoration(resources.push_constant_buffers[i].id, spv::DecorationOffset);
            push_constant.stageFlags = shader_stage;
            push_constants.emplace_back(push_constant);
        }
    }

    struct ResourceDescriptorType {
        VkDescriptorType type;
        spirv_cross::SmallVector<spirv_cross::Resource> resources;
    };

    // Supported descriptor types
    std::vector<ResourceDescriptorType> descriptor_type_arrays = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, resources.uniform_buffers }, { VK_DESCRIPTOR_TYPE_SAMPLER, resources.separate_samplers },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, resources.separate_images },  { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.sampled_images },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resources.storage_buffers }, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, resources.storage_images },
    };

    for (const auto& [descriptor_type, descriptor_array] : descriptor_type_arrays) {
        for (u32 descriptor_index = 0; descriptor_index < descriptor_array.size(); descriptor_index++) {
            auto& element_type = compiler.get_type(descriptor_array[descriptor_index].base_type_id);

            u32 set_index = compiler.get_decoration(descriptor_array[descriptor_index].id, spv::DecorationDescriptorSet);
            TK_ASSERT(set_index <= MAX_DESCRIPTOR_SETS, "A maximum of {} descriptor sets supported (indices 0 - {})", MAX_DESCRIPTOR_SETS, MAX_DESCRIPTOR_SETS);
            TK_ASSERT(bindings.binding_counts[set_index] <= MAX_DESCRIPTOR_BINDINGS, "A maximum of {} descriptor set bindings supported", MAX_DESCRIPTOR_BINDINGS);

            u32 binding_index = compiler.get_decoration(descriptor_array[descriptor_index].id, spv::DecorationBinding);

            bindings.bindings[set_index + bindings.binding_counts[set_index]].binding = binding_index;
            bindings.bindings[set_index + bindings.binding_counts[set_index]].descriptorCount = element_type.array.size() == 0 ? 1 : element_type.array[0];
            bindings.bindings[set_index + bindings.binding_counts[set_index]].descriptorType = descriptor_type;
            bindings.bindings[set_index + bindings.binding_counts[set_index]].stageFlags |= shader_stage;

            bindings.binding_counts[set_index]++;
        }
    }
}

VulkanGraphicsPipeline::PipelineResources VulkanGraphicsPipeline::create_pipeline_resources(Ref<RendererContext> ctx, const std::vector<configs::Shader>& stages) {
    PipelineResources resources;

    DescriptorBindings bindings;
    std::vector<VkPushConstantRange> push_constants;
    for (const auto& shader : stages) {
        std::string shader_source = loaders::read_text_file("assets/shaders" / shader.path);
        std::vector shader_binary = compile_shader(shader.stage, shader_source);
        resources.shader_modules[shader.stage] = create_shader_module(ctx, shader_binary);
        reflect_shader(shader.stage, shader_binary, bindings, push_constants);
    }

    PipelineLayoutCreateConfig pipeline_layout_config{ .bindings = bindings, .push_constants = push_constants };
    resources.layout = create_pipeline_layout(ctx, pipeline_layout_config);

    return resources;
}

}  // namespace toki
