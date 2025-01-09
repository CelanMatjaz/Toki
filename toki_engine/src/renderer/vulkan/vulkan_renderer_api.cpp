#include "vulkan_renderer_api.h"

#include "core/assert.h"

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

    if (i32 index = framebuffer.get_present_target_index(); index >= 0) {
        color_attachment_infos[index].imageView = m_context->swapchain.get_current_frame_image_view();
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

    m_isPassStarted = true;
};

void VulkanRendererApi::end_pass() {
    TK_ASSERT(m_isPassStarted, "No render pass was started");
    vkCmdDraw(m_context->swapchain.get_current_command_buffer(), 6, 1, 0, 0);
    vkCmdEndRendering(m_context->swapchain.get_current_command_buffer());

    m_isPassStarted = false;
}

void VulkanRendererApi::submit() {
    m_context->swapchain.transition_current_frame_image();
}

void VulkanRendererApi::bind_shader(Handle shader_handle) {
    TK_ASSERT(m_context->shaders.contains(shader_handle), "Shader with provided handle does not exist");
    VulkanGraphicsPipeline& pipeline = m_context->shaders.get(shader_handle);
    vkCmdBindPipeline(m_context->swapchain.get_current_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_handle());
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

void VulkanRendererApi::fix_render_area(Rect2D& render_area) {
    if (render_area.size.x == 0 || render_area.size.y == 0) {
        Vec2 window_dimensions{ m_context->swapchain.get_extent().width, m_context->swapchain.get_extent().height };
        render_area.size.x = window_dimensions.x;
        render_area.size.y = window_dimensions.y;
    }
}

}  // namespace toki
