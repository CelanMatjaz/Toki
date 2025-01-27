#include "renderer.h"

#include "renderer/vulkan/vulkan_backend.h"

namespace toki {

#define backend reinterpret_cast<vulkan_renderer::VulkanBackend*>(m_backend)

Renderer::Renderer(const Config& config): m_backend(new vulkan_renderer::VulkanBackend()) {
    backend->create_device(config.initial_window);
    backend->create_swapchain(config.initial_window);
    backend->create_render_pass();
    backend->initialize_resources();
}

void Renderer::begin_frame() {
    backend->prepare_frame_resources();
}

void Renderer::end_frame() {
    backend->submit_commands();
    backend->cleanup_frame_resources();
}

void Renderer::present() {
    backend->present();
}

void Renderer::submit(SubmitFn submit_fn) {
    RendererCommands* commands = backend->get_commands();
    submit_fn(*commands);
}

void Renderer::set_color_clear(const glm::vec4& color) {
    backend->set_color_clear(color);
}

void Renderer::set_depth_clear(f32 depth) {}

RenderPass Renderer::create_render_pass() {
    return {};
}

void Renderer::destroy_render_pass(RenderPass& render_pass) {}

Buffer Renderer::create_buffer(BufferType type, u32 size) {
    return backend->create_buffer(type, size);
}

void Renderer::destroy_buffer(Buffer& buffer) {
    backend->destroy_buffer(&buffer);
}

Texture Renderer::create_texture(ColorFormat format, u32 width, u32 height) {
    Texture new_texture{};
    new_texture.handle = backend->create_image(format, width, height);
    return new_texture;
}

void Renderer::destroy_texture(Handle texture_handle) {
    backend->destroy_image(texture_handle);
}

Shader Renderer::create_shader(RenderPass& render_pass, configs::ShaderConfig shader_config) {
    return backend->create_pipeline(render_pass.handle, shader_config);
}

void Renderer::destroy_shader(Shader& shader) {}

void Renderer::wait_for_resources() {
    backend->wait_for_resources();
}

#undef backend

}  // namespace toki
