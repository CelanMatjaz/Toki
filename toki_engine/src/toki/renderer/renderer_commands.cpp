#include "renderer_commands.h"

#include "renderer/vulkan/vulkan_backend.h"

namespace toki {

#define backend reinterpret_cast<vulkan_renderer::VulkanBackend*>(m_backend)

void RendererCommands::begin_pass() {}

void RendererCommands::end_pass() {}

void RendererCommands::draw(u32 count) {}

void RendererCommands::draw_indexed(u32 count) {}

void RendererCommands::bind_shader(Shader const& shader) {}

void RendererCommands::bind_buffer(Buffer const& buffer) {}

#undef backend

}  // namespace toki
