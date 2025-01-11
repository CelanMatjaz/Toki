#include "vulkan_renderer_api.h"

#include <cstring>

#include "core/assert.h"
#include "vulkan/vulkan_core.h"

namespace toki {

VulkanRendererApi::VulkanRendererApi(Ref<RendererContext> context): m_context(context) {}

VulkanRendererApi::~VulkanRendererApi() {
    m_context.reset();
}

void VulkanRendererApi::begin_pass(const BeginPassConfig& config) {
    TK_ASSERT(!m_isPassStarted, "A render pass was already started");
    TK_ASSERT(m_context->framebuffers.contains(config.framebufferHandle), "Cannot begin render pass without an existing framebuffer");

    Rect2D render_area = config.renderArea;
    fix_render_area(render_area);

    VulkanFramebuffer& framebuffer = m_context->framebuffers.get(config.framebufferHandle);
    std::vector color_attachment_infos = framebuffer.get_color_attachments_rendering_infos();

    i32 present_index = framebuffer.get_present_target_index();
    if (present_index >= 0) {
        color_attachment_infos[present_index].imageView = m_context->swapchain.get_current_frame_image_view();
        color_attachment_infos[present_index].clearValue.color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        m_context->swapchain.prepare_current_frame_image();
    }
    for (auto& attachment : color_attachment_infos) {
        memset(attachment.clearValue.color.float32, 0, sizeof(attachment.clearValue.color.float32));
    }

    VkRenderingInfo rendering_info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    rendering_info.renderArea.extent.width = render_area.size.x;
    rendering_info.renderArea.extent.height = render_area.size.y;
    rendering_info.renderArea.offset.x = render_area.pos.x;
    rendering_info.renderArea.offset.y = render_area.pos.y;
    rendering_info.layerCount = 1;
    rendering_info.pColorAttachments = color_attachment_infos.data();
    rendering_info.colorAttachmentCount = color_attachment_infos.size();

    vkCmdBeginRendering(m_context->swapchain.get_current_command_buffer(), &rendering_info);

    vkCmdPushConstants(m_context->swapchain.get_current_command_buffer(), m_context->shaders.begin()->get_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &config.viewProjectionMatrix);

    m_isPassStarted = true;
};

void VulkanRendererApi::end_pass() {
    TK_ASSERT(m_isPassStarted, "No render pass was started");
    vkCmdEndRendering(m_context->swapchain.get_current_command_buffer());

    m_isPassStarted = false;
}

void VulkanRendererApi::submit() {}

void VulkanRendererApi::bind_shader(Handle shader_handle) {
    TK_ASSERT(m_context->shaders.contains(shader_handle), "Shader with provided handle does not exist");
    VulkanGraphicsPipeline& pipeline = m_context->shaders.get(shader_handle);
    vkCmdBindPipeline(m_context->swapchain.get_current_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_handle());
}

void VulkanRendererApi::bind_vertex_buffer(Handle handle) {
    TK_ASSERT(m_context->buffers.contains(handle), "Buffer with provided handle does not exist");
    VkBuffer buf = m_context->buffers[handle].get_handle();
    u64 offsets = 0;
    vkCmdBindVertexBuffers(m_context->swapchain.get_current_command_buffer(), 0, 1, &buf, &offsets);
}

void VulkanRendererApi::bind_vertex_buffer(Handle handle, u32 binding) {
    TK_ASSERT(m_context->buffers.contains(handle), "Buffer with provided handle does not exist");
    VkBuffer buf = m_context->buffers[handle].get_handle();
    u64 offsets = 0;
    vkCmdBindVertexBuffers(m_context->swapchain.get_current_command_buffer(), binding, 1, &buf, &offsets);
}

void VulkanRendererApi::bind_vertex_buffers(const BindVertexBuffersConfig& config) {
    std::vector<VkBuffer> buffers(config.handles.size());
    std::vector<VkDeviceSize> offsets(config.handles.size(), 0);
    u64 offset = 0;
    for (u32 i = 0; i < buffers.size(); i++) {
        TK_ASSERT(m_context->buffers.contains(config.handles[i]), "Buffer with provided handle does not exist");
        VkBuffer buffer = m_context->buffers.get(config.handles[i]).get_handle();
        vkCmdBindVertexBuffers(m_context->swapchain.get_current_command_buffer(), i, 1, &buffer, &offset);
    }
}

void VulkanRendererApi::bind_index_buffer(Handle handle) {
    TK_ASSERT(m_context->buffers.contains(handle), "Buffer with provided handle does not exist");
    VkBuffer buf = m_context->buffers[handle].get_handle();
    vkCmdBindIndexBuffer(m_context->swapchain.get_current_command_buffer(), buf, 0, VK_INDEX_TYPE_UINT32);
}

void VulkanRendererApi::reset_viewport() {
    Vec2 window_dimensions{ m_context->swapchain.get_extent().width, m_context->swapchain.get_extent().height };

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = window_dimensions.x;
    viewport.height = window_dimensions.y;
    viewport.minDepth = 0.0f;
    viewport.minDepth = 1.0f;

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
