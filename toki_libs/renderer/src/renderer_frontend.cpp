#include "renderer_frontend.h"

#include "vulkan/vulkan_backend.h"

namespace toki {

// QoL macro
#define backend reinterpret_cast<VulkanBackend*>(mBackend)

RendererFrontend::RendererFrontend([[maybe_unused]] const Config& config): mBackend(new VulkanBackend()) {}

RendererFrontend::~RendererFrontend() {
    backend->~VulkanBackend();
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

void RendererFrontend::submit(SubmitFunctionConcept auto submit_fn) {
    RendererCommands* commands = backend->get_commands();
    submit_fn(*commands);
}

void RendererFrontend::set_color_clear(const Vec4<f32>& color) {
    backend->set_color_clear(color);
}

void RendererFrontend::set_depth_clear(f32 depth) {
    backend->set_depth_clear(depth);
}

Framebuffer RendererFrontend::framebuffer_create(const FramebufferConfig& config) {
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

void RendererFrontend::framebuffer_destroy(Framebuffer& framebuffer) {
    backend->destroy_framebuffer(framebuffer.handle);
}

Buffer RendererFrontend::buffer_create(const BufferConfig& config) {
    Buffer new_buffer{};
    new_buffer.handle = backend->create_buffer(config.type, config.size);
    return new_buffer;
}

void RendererFrontend::buffer_destroy(Buffer& buffer) {
    backend->destroy_buffer(buffer);
}

void RendererFrontend::buffer_set_data(const Buffer& buffer, u32 size, void* data) {
    backend->set_buffer_data(buffer, size, data);
}

Texture RendererFrontend::texture_create(const TextureConfig& config) {
    Texture new_texture{};
    new_texture.handle = backend->create_image(config.format, config.width, config.height);
    return new_texture;
}

void RendererFrontend::texture_destroy(Texture& texture) {
    backend->destroy_image(texture.handle);
}

Shader RendererFrontend::shader_create(const Framebuffer& framebuffer, const ShaderConfig& config) {
    Shader shader{};
    shader.handle = backend->create_shader_internal(framebuffer, config);
    return shader;
}

void RendererFrontend::shader_destroy(Shader& shader) {
    backend->destroy_shader_internal(shader.handle);
}

void RendererFrontend::wait_for_resources() {
    backend->wait_for_resources();
}

}  // namespace toki
