#include "renderer_frontend.h"

#include "vulkan/vulkan_backend.h"

namespace toki {

// QoL macro
#define backend reinterpret_cast<VulkanBackend*>(mBackend)

RendererFrontend::RendererFrontend([[maybe_unused]] const Config& config): mBackend(new VulkanBackend()) {}

RendererFrontend::~RendererFrontend() {
    backend->~VulkanBackend();
}

void RendererFrontend::frame_begin() {
    backend->prepare_frame_resources();
}

void RendererFrontend::frame_end() {
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

void RendererFrontend::window_add(NativeWindowHandle handle) {
    backend->swapchain_create(handle);
}

void RendererFrontend::set_color_clear(const Vec4<f32>& color) {
    backend->set_color_clear(color);
}

void RendererFrontend::set_depth_clear(f32 depth) {
    backend->set_depth_clear(depth);
}

Framebuffer RendererFrontend::framebuffer_create(const FramebufferConfig& config) {
    Framebuffer new_framebuffer{};
    new_framebuffer.handle = backend->framebuffer_create(config);
    return new_framebuffer;
}

void RendererFrontend::framebuffer_destroy(Framebuffer& framebuffer) {
    backend->framebuffer_destroy(framebuffer.handle);
}

Buffer RendererFrontend::buffer_create(const BufferConfig& config) {
    Buffer new_buffer{};
    new_buffer.handle = backend->buffer_create(config);
    return new_buffer;
}

void RendererFrontend::buffer_destroy(Buffer& buffer) {
    backend->buffer_destroy(buffer.handle);
}

void RendererFrontend::buffer_set_data(const Buffer& buffer, u32 size, void* data) {
    backend->buffer_set_data(buffer, size, data);
}

Texture RendererFrontend::texture_create(const TextureConfig& config) {
    Texture new_texture{};
    new_texture.handle = backend->image_create(config);
    return new_texture;
}

void RendererFrontend::texture_destroy(Texture& texture) {
    backend->image_destroy(texture.handle);
}

Shader RendererFrontend::shader_create(const Framebuffer& framebuffer, const ShaderConfig& config) {
    Shader shader{};
    shader.handle = backend->shader_create(framebuffer, config);
    return shader;
}

void RendererFrontend::shader_destroy(Shader& shader) {
    backend->shader_destroy(shader.handle);
}

void RendererFrontend::wait_for_resources() {
    backend->resources_wait();
}

}  // namespace toki
