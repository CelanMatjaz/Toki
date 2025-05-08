#include "vulkan_commands.h"

#include <vulkan/vulkan.h>

#include "vulkan_backend.h"

namespace toki {

#define backend m_backend

VulkanCommands::VulkanCommands(VulkanBackend* b): m_backend(b), m_command_buffer(b->get_command_buffer()) {}

void VulkanCommands::begin_rendering(Framebuffer framebuffer, const Rect2D& render_area) {
	m_bound_framebuffer = framebuffer;
	backend->begin_rendering(m_command_buffer, framebuffer, render_area);
}

void VulkanCommands::end_rendering() {
	backend->end_rendering(m_command_buffer, m_bound_framebuffer);
}

void VulkanCommands::set_viewport(const Rect2D& rect) {
	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (f32) rect.size.x;
	viewport.height = (f32) rect.size.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_command_buffer, 0, 1, &viewport);
}

void VulkanCommands::reset_viewport() {
	set_viewport({ { 0, 0 }, { 600, 600 } });
}

void VulkanCommands::set_scissor(const Rect2D& rect) {
	VkRect2D scissor{};
	scissor.offset = { static_cast<i32>(rect.pos.x), static_cast<i32>(rect.pos.y) };
	scissor.extent = { static_cast<u32>(rect.size.x), static_cast<u32>(rect.size.y) };
	vkCmdSetScissor(m_command_buffer, 0, 1, &scissor);
}

void VulkanCommands::reset_scissor() {
	set_scissor(Rect2D{ { 0, 0 }, { 600, 600 } });
}

void VulkanCommands::bind_shader(Shader shader) {
	m_bound_shader = shader;
	backend->bind_shader(m_command_buffer, shader);
}

void VulkanCommands::bind_buffer(Buffer buffer) {
	backend->bind_buffer(m_command_buffer, buffer);
}

void VulkanCommands::bind_texture(Texture buffer) {}

void VulkanCommands::push_constants(u32 offset, u32 size, const void* data) {
	InternalShader* shader = backend->get_shader(m_bound_shader);
	VkPipelineLayout pipeline_layout = shader->pipeline_layout;
	VkShaderStageFlags shader_stage_flags = shader->push_constant_stage_flags;
	backend->push_constants(m_command_buffer, pipeline_layout, shader_stage_flags, offset, size, data);
}

void VulkanCommands::draw(u32 vertex_count) {
	m_backend->draw(m_command_buffer, vertex_count);
}

void VulkanCommands::draw_indexed(u32 index_count) {
	m_backend->draw_indexed(m_command_buffer, index_count);
}

void VulkanCommands::draw_instanced(u32 index_count, u32 instance_count) {
	m_backend->draw_instanced(m_command_buffer, index_count, instance_count);
}

}  // namespace toki
