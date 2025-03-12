#include "renderer_frontend.h"

#include "vulkan/vulkan_backend.h"

namespace toki {

namespace renderer {

// QoL macro
#define backend reinterpret_cast<renderer::VulkanBackend*>(m_backend)

RendererFrontend::RendererFrontend(const Config& config): m_backend(new renderer::VulkanBackend()) {
    backend->create_device(config.initial_window);
    backend->create_swapchain(config.initial_window);
    backend->initialize_resources();
}

void RendererFrontend::begin_frame() {
    backend->prepare_frame_resources();
}

void RendererFrontend::end_frame() {
    backend->cleanup_frame_resources();
}

void RendererFrontend::present() {
    backend->submit_commands();
    backend->present();
}

void RendererFrontend::submit(SubmitFn submit_fn) {
    RendererCommands* commands = backend->get_commands();
    submit_fn(*commands);
}

void RendererFrontend::set_color_clear(const glm::vec4& color) {
    backend->set_color_clear(color);
}

void RendererFrontend::set_depth_clear(f32 depth) {
    backend->set_depth_clear(depth);
}

Framebuffer RendererFrontend::create_framebuffer(const FramebufferConfig& config) {
    Framebuffer new_framebuffer{};
    new_framebuffer.handle = backend->create_framebuffer(
        config.image_width,
        config.image_height,
        config.color_attachment_count,
        config.color_format,
        config.has_depth_attachment,
        config.has_stencil_attachment);
    return new_framebuffer;
}

void RendererFrontend::destroy_framebuffer(Framebuffer* framebuffer) {
    backend->destroy_framebuffer(framebuffer->handle);
}

Buffer RendererFrontend::create_buffer(const BufferConfig& config) {
    Buffer new_buffer{};
    new_buffer.handle = backend->create_buffer(config.type, config.size);
    new_buffer.size = config.size;
    new_buffer.type = config.type;
    return new_buffer;
}

void RendererFrontend::destroy_buffer(Buffer* buffer) {
    backend->destroy_buffer(buffer);
}

void RendererFrontend::set_bufffer_data(Buffer* buffer, u32 size, void* data) {
    backend->set_buffer_data(buffer, size, data);
}

Texture RendererFrontend::create_texture(const TextureConfig& config) {
    Texture new_texture{};
    new_texture.handle = backend->create_image(config.format, config.width, config.height);
    new_texture.width = config.width;
    new_texture.height = config.height;
    return new_texture;
}

void RendererFrontend::destroy_texture(Texture* texture) {
    backend->destroy_image(texture->handle);
}

Shader RendererFrontend::create_shader(const Framebuffer* framebuffer, const ShaderConfig& config) {
    Shader shader{};
    shader.handle = backend->create_shader_internal(framebuffer, config);
    return shader;
}

void RendererFrontend::destroy_shader(Shader* shader) {
    backend->destroy_shader_internal(shader->handle);
}

void RendererFrontend::wait_for_resources() {
    backend->wait_for_resources();
}

#undef backend

}  // namespace renderer

}  // namespace toki
