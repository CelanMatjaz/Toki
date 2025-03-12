#include "vulkan_commands.h"

#include <vulkan/vulkan.h>

#include "vulkan_backend.h"

namespace toki {

namespace renderer {

#define backend m_backend

VulkanCommands::VulkanCommands(VulkanBackend* b): m_backend(b), m_commandBuffer(b->get_command_buffer()) {}

void VulkanCommands::begin_rendering(const Framebuffer* framebuffer, const Rect2D& render_area) {
    m_framebufferHandle = framebuffer->handle;
    backend->begin_rendering(m_commandBuffer, framebuffer->handle, render_area);
}

void VulkanCommands::end_rendering() {
    backend->end_rendering(m_commandBuffer, m_framebufferHandle);
}

void VulkanCommands::set_viewport(const Rect2D& rect) {
    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (f32) rect.size.x;
    viewport.height = (f32) rect.size.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
}

void VulkanCommands::reset_viewport() {
    set_viewport({ { 0, 0 }, { 600.0f, 600.0f } });
}

void VulkanCommands::set_scissor(const Rect2D& rect) {
    VkRect2D scissor{};
    scissor.offset = { rect.pos.x, rect.pos.y };
    scissor.extent = { rect.size.x, rect.size.y };
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void VulkanCommands::reset_scissor() {
    set_scissor(Rect2D{ { 0, 0 }, { 600, 600 } });
}

void VulkanCommands::bind_shader(Shader const& shader) {
    m_shaderHandle = shader.handle;
    backend->bind_shader(m_commandBuffer, shader);
}

void VulkanCommands::bind_buffer(Buffer const& buffer) {
    backend->bind_buffer(m_commandBuffer, buffer);
}

void VulkanCommands::push_constants(u32 offset, u32 size, const void* data) {
    InternalShader* shader = backend->get_shader(m_shaderHandle);
    VkPipelineLayout pipeline_layout = shader->pipelines[m_shaderHandle.data].pipeline_layout;
    VkShaderStageFlags shader_stage_flags = shader->push_constant_stage_flags;
    backend->push_constants(m_commandBuffer, pipeline_layout, shader_stage_flags, offset, size, data);
}

void VulkanCommands::draw(u32 vertex_count) {
    m_backend->draw(m_commandBuffer, vertex_count);
}

void VulkanCommands::draw_indexed(u32 index_count) {
    m_backend->draw_indexed(m_commandBuffer, index_count);
}

void VulkanCommands::draw_instanced(u32 index_count, u32 instance_count) {
    m_backend->draw_instanced(m_commandBuffer, index_count, instance_count);
}

}  // namespace renderer

}  // namespace toki
