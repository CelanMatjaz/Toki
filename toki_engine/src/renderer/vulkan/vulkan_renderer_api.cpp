#include "vulkan_renderer_api.h"

#include "core/assert.h"

namespace toki {

VulkanRendererApi::VulkanRendererApi(Ref<RendererContext> context): _context(context) {}

VulkanRendererApi::~VulkanRendererApi() {
    _context.reset();
}

void VulkanRendererApi::begin_pass(const BeginPassConfig& config) {
    TK_ASSERT(_context->framebuffers.contains(config.framebufferHandle), "Cannot begin render pass without an existing framebuffer");

    Rect2D render_area = config.renderArea;
    fix_render_area(render_area);

    VulkanFramebuffer& framebuffer = _context->framebuffers.get(config.framebufferHandle);
    std::vector color_attachment_infos = framebuffer.get_color_attachments_rendering_infos();

    if (i32 index = framebuffer.get_present_target_index(); index >= 0) {
        color_attachment_infos[index].imageView = _context->swapchain.get_current_frame_image_view();
    }

    VkRenderingInfo rendering_info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    rendering_info.renderArea.extent.width = render_area.size.width;
    rendering_info.renderArea.extent.height = render_area.size.height;
    rendering_info.renderArea.offset.x = render_area.pos.x;
    rendering_info.renderArea.offset.y = render_area.pos.y;
    rendering_info.layerCount = 1;
    rendering_info.pColorAttachments = color_attachment_infos.data();
    rendering_info.colorAttachmentCount = color_attachment_infos.size();

    vkCmdBeginRendering(_context->swapchain.get_current_command_buffer(), &rendering_info);
};

void VulkanRendererApi::end_pass() {
    vkCmdDraw(_context->swapchain.get_current_command_buffer(), 6, 1, 0, 0);
    vkCmdEndRendering(_context->swapchain.get_current_command_buffer());
}

void VulkanRendererApi::submit() {
    _context->swapchain.transition_current_frame_image();
}

void VulkanRendererApi::bind_shader(Handle shader_handle) {
    TK_ASSERT(_context->shaders.contains(shader_handle), "Shader with provided handle does not exist");
    VulkanGraphicsPipeline& pipeline = _context->shaders.get(shader_handle);
    vkCmdBindPipeline(_context->swapchain.get_current_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_handle());
}

void VulkanRendererApi::reset_viewport() {
    Vec2i window_dimensions{ _context->swapchain.get_extent().width, _context->swapchain.get_extent().height };

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = window_dimensions.width;
    viewport.height = window_dimensions.height;
    viewport.minDepth = 0.0f;
    viewport.minDepth = 1.0f;

    vkCmdSetViewport(_context->swapchain.get_current_command_buffer(), 0, 1, &viewport);
}

void VulkanRendererApi::reset_scissor() {
    Vec2i window_dimensions{ _context->swapchain.get_extent().width, _context->swapchain.get_extent().height };

    VkRect2D scissor{};
    scissor.extent.width = window_dimensions.width;
    scissor.extent.height = window_dimensions.height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;

    vkCmdSetScissor(_context->swapchain.get_current_command_buffer(), 0, 1, &scissor);
}

void VulkanRendererApi::fix_render_area(Rect2D& render_area) {
    if (render_area.size.width == 0 || render_area.size.height == 0) {
        Vec2i window_dimensions{ _context->swapchain.get_extent().width, _context->swapchain.get_extent().height };
        render_area.size.width = window_dimensions.width;
        render_area.size.height = window_dimensions.height;
    }
}

}  // namespace toki
