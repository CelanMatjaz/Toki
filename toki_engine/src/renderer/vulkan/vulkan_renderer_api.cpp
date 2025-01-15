#include "vulkan_renderer_api.h"

#include "core/assert.h"
#include "renderer/vulkan/vulkan_utils.h"
#include "vulkan/vulkan_core.h"

namespace toki {

#define CHECK_SHADER(shader_handle) TK_ASSERT(m_context->shaders.contains(shader_handle), "Shader with provided handle does not exist");
#define CHECK_FRAMEBUFFER(framebuffer_handle) TK_ASSERT(m_context->framebuffers.contains(framebuffer_handle), "Framebuffer with provided handle does not exist");
#define CHECK_BUFFER(buffer_handle) TK_ASSERT(m_context->buffers.contains(buffer_handle), "Buffer with provided handle does not exist");
#define CHECK_TEXTURE(texture_handle) TK_ASSERT(m_context->images.contains(texture_handle), "Texture with provided handle does not exist");

VulkanRendererApi::VulkanRendererApi(Ref<RendererContext> context): m_context(context) {}

VulkanRendererApi::~VulkanRendererApi() {
    m_context.reset();
}

void VulkanRendererApi::begin_pass(const BeginPassConfig& config) {
    TK_ASSERT(!m_isPassStarted, "A render pass was already started");
    TK_ASSERT(m_context->framebuffers.contains(config.framebufferHandle), "Cannot begin render pass without an existing framebuffer");

    VkCommandBuffer cmd = m_context->swapchain.get_current_command_buffer();

    Rect2D render_area = config.renderArea;
    fix_render_area(render_area);

    static std::vector<VkRenderingAttachmentInfo> color_attachment_infos(MAX_FRAMEBUFFER_ATTACHMENTS);
    VulkanFramebuffer& framebuffer = m_context->framebuffers.at(config.framebufferHandle);
    const std::vector<RenderTarget> render_targets = framebuffer.get_render_target_configs();
    const std::vector<VkFormat>& color_formats = framebuffer.get_color_formats();
    const std::vector<VulkanImage>& images = framebuffer.get_images();

    i32 present_index = framebuffer.get_present_target_index();
    for (u32 i = 0; i < color_formats.size(); i++) {
        auto& info = color_attachment_infos[i];
        color_attachment_infos[i] = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        color_attachment_infos[i].storeOp = map_attachment_store_op(render_targets[i].storeOp);
        color_attachment_infos[i].loadOp = map_attachment_load_op(render_targets[i].loadOp);
        color_attachment_infos[i].imageLayout = get_image_layout(render_targets[i].colorFormat);

        if (i == present_index) {
            m_context->swapchain.prepare_current_frame_image();
            color_attachment_infos[i].imageView = m_context->swapchain.get_current_frame_image_view();
        } else {
            color_attachment_infos[i].imageView = images[i].get_image_view();
        }
    }

    VkRenderingInfo rendering_info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    rendering_info.renderArea.extent.width = render_area.size.x;
    rendering_info.renderArea.extent.height = render_area.size.y;
    rendering_info.renderArea.offset.x = render_area.pos.x;
    rendering_info.renderArea.offset.y = render_area.pos.y;
    rendering_info.layerCount = 1;
    rendering_info.pColorAttachments = color_attachment_infos.data();
    rendering_info.colorAttachmentCount = color_formats.size();

    VkRenderingAttachmentInfo depth_attachment_info{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    if (const auto& depth_image = framebuffer.get_depth_image(); depth_image) {
        depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth_attachment_info.imageView = depth_image->get_image_view();
        depth_attachment_info.clearValue.depthStencil = { 1.0f, 0 };
        rendering_info.pDepthAttachment = &depth_attachment_info;
    }

    framebuffer.transition_images(cmd);

    vkCmdBeginRendering(m_context->swapchain.get_current_command_buffer(), &rendering_info);

    m_isPassStarted = true;
};

void VulkanRendererApi::end_pass() {
    TK_ASSERT(m_isPassStarted, "No render pass was started");
    vkCmdEndRendering(m_context->swapchain.get_current_command_buffer());

    m_isPassStarted = false;
}

void VulkanRendererApi::submit() {}

void VulkanRendererApi::bind_shader(Handle shader_handle) {
    CHECK_SHADER(shader_handle);
    VulkanGraphicsPipeline& pipeline = m_context->shaders.at(shader_handle);
    vkCmdBindPipeline(m_context->swapchain.get_current_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_handle());
}

void VulkanRendererApi::bind_vertex_buffer(Handle buffer_handle) {
    CHECK_BUFFER(buffer_handle)
    VkBuffer buf = m_context->buffers.at(buffer_handle).get_handle();
    u64 offsets = 0;
    vkCmdBindVertexBuffers(m_context->swapchain.get_current_command_buffer(), 0, 1, &buf, &offsets);
}

void VulkanRendererApi::bind_vertex_buffer(Handle buffer_handle, u32 binding) {
    CHECK_BUFFER(buffer_handle)
    VkBuffer buf = m_context->buffers.at(buffer_handle).get_handle();
    u64 offsets = 0;
    vkCmdBindVertexBuffers(m_context->swapchain.get_current_command_buffer(), binding, 1, &buf, &offsets);
}

void VulkanRendererApi::bind_vertex_buffers(const BindVertexBuffersConfig& config) {
    std::vector<VkBuffer> buffers(config.handles.size());
    std::vector<VkDeviceSize> offsets(config.handles.size(), 0);
    u64 offset = 0;
    for (u32 i = 0; i < buffers.size(); i++) {
        CHECK_BUFFER(config.handles[i]);
        VkBuffer buf = m_context->buffers.at(config.handles[i]).get_handle();
        vkCmdBindVertexBuffers(m_context->swapchain.get_current_command_buffer(), i, 1, &buf, &offset);
    }
}

void VulkanRendererApi::bind_index_buffer(Handle buffer_handle) {
    CHECK_BUFFER(buffer_handle)
    VkBuffer buf = m_context->buffers.at(buffer_handle).get_handle();
    vkCmdBindIndexBuffer(m_context->swapchain.get_current_command_buffer(), buf, 0, VK_INDEX_TYPE_UINT32);
}

void VulkanRendererApi::push_constant(Handle shader_handle, u32 size, void* data) {
    CHECK_SHADER(shader_handle);
    VulkanGraphicsPipeline& pipeline = m_context->shaders.at(shader_handle);
    vkCmdPushConstants(m_context->swapchain.get_current_command_buffer(), pipeline.get_layout(), pipeline.get_push_constant_stage_bits(), 0, size, data);

    // vkCmdBindDescriptorSets(
    //     VkCommandBuffer commandBuffer,
    //     VkPipelineBindPoint pipelineBindPoint,
    //     VkPipelineLayout layout,
    //     uint32_t firstSet,
    //     uint32_t descriptorSetCount,
    //     const VkDescriptorSet* pDescriptorSets,
    //     uint32_t dynamicOffsetCount,
    //     const uint32_t* pDynamicOffsets)
}

void VulkanRendererApi::update_sets(Handle shader_handle) {
    CHECK_SHADER(shader_handle);
    VulkanGraphicsPipeline& pipeline = m_context->shaders.at(shader_handle);
    Frame& frame = m_context->swapchain.get_current_frame();
    for (const auto set : pipeline.get_descriptor_sets()) {
        frame.descriptor_writer.update_set(m_context, set);
    }
    frame.descriptor_writer.clear();
}

void VulkanRendererApi::bind_descriptor_sets(Handle shader_handle) {
    CHECK_SHADER(shader_handle);
    VulkanGraphicsPipeline& pipeline = m_context->shaders.at(shader_handle);
    std::vector descriptor_sets = pipeline.get_descriptor_sets();
    vkCmdBindDescriptorSets(m_context->swapchain.get_current_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_layout(), 0, descriptor_sets.size(), descriptor_sets.data(), 0, nullptr);
}

void VulkanRendererApi::reset_descriptor_sets(Handle shader_handle) {
    CHECK_SHADER(shader_handle);
    VulkanGraphicsPipeline& pipeline = m_context->shaders.at(shader_handle);
    Frame& frame = m_context->swapchain.get_current_frame();
    frame.descriptor_writer.clear();

    pipeline.allocate_descriptor_sets(m_context, frame.descriptor_pool_manager);
}

void VulkanRendererApi::write_buffer(Handle shader_handle, Handle buffer_handle, u32 set, u32 binding) {
    CHECK_SHADER(shader_handle);
    CHECK_BUFFER(buffer_handle);
    VulkanGraphicsPipeline& pipeline = m_context->shaders.at(shader_handle);
    VulkanBuffer& buffer = m_context->buffers.at(buffer_handle);
    Frame& frame = m_context->swapchain.get_current_frame();
    pipeline.allocate_descriptor_sets(m_context, frame.descriptor_pool_manager);

    DescriptorWriter::WriteBufferConfig config{};
    config.buffer = buffer.get_handle();
    config.size = buffer.get_size();
    config.offset = 0;
    frame.descriptor_writer.write_buffer(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, config);
}

void VulkanRendererApi::write_texture(Handle shader_handle, Handle texture_handle, u32 set, u32 binding) {
    CHECK_SHADER(shader_handle);
    CHECK_TEXTURE(texture_handle);

    VulkanGraphicsPipeline& pipeline = m_context->shaders.at(shader_handle);
    VulkanImage& image = m_context->images.at(texture_handle);
    Frame& frame = m_context->swapchain.get_current_frame();
    pipeline.allocate_descriptor_sets(m_context, frame.descriptor_pool_manager);

    DescriptorWriter::WriteImageConfig config{};
    config.image_view = image.get_image_view();
    config.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    config.sampler = m_context->default_sampler;
    frame.descriptor_writer.write_image(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, config);
}

void VulkanRendererApi::reset_viewport() {
    Vec2 window_dimensions{ m_context->swapchain.get_extent().width, m_context->swapchain.get_extent().height };

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = window_dimensions.x;
    viewport.height = window_dimensions.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(m_context->swapchain.get_current_command_buffer(), 0, 1, &viewport);
}

void VulkanRendererApi::reset_scissor() {
    Vec2 window_dimensions{ m_context->swapchain.get_extent().width, m_context->swapchain.get_extent().height };

    VkRect2D scissor{};
    scissor.extent.width = window_dimensions.x;
    scissor.extent.height = window_dimensions.y;
    scissor.offset.x = 0;
    scissor.offset.y = 0;

    vkCmdSetScissor(m_context->swapchain.get_current_command_buffer(), 0, 1, &scissor);
}

void VulkanRendererApi::draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) {
    vkCmdDraw(m_context->swapchain.get_current_command_buffer(), vertex_count, instance_count, first_vertex, first_instance);
}

void VulkanRendererApi::draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) {
    vkCmdDrawIndexed(m_context->swapchain.get_current_command_buffer(), index_count, instance_count, first_index, vertex_offset, first_instance);
}

void VulkanRendererApi::fix_render_area(Rect2D& render_area) {
    if (render_area.size.x == 0 || render_area.size.y == 0) {
        Vec2 window_dimensions{ m_context->swapchain.get_extent().width, m_context->swapchain.get_extent().height };
        render_area.size.x = window_dimensions.x;
        render_area.size.y = window_dimensions.y;
    }
}

}  // namespace toki
