#pragma once

#include <toki/core.h>

namespace toki {

class RendererBackend {
public:
    Handle create_framebuffer(
        u32 width, u32 height, u32 color_attachment_count, ColorFormat format, b8 has_depth, b8 has_stencil);
    void destroy_framebuffer(Handle framebuffer_handle);

    Handle create_buffer(BufferType type, u32 size);
    void destroy_buffer(Buffer* buffer);
    void* map_buffer_memory(VkDeviceMemory memory, u32 offset, u32 size);
    void unmap_buffer_memory(VkDeviceMemory memory);
    void flush_buffer(Buffer* buffer);
    void set_buffer_data(Buffer* buffer, u32 size, void* data);
    void copy_buffer_data(VkBuffer dst, VkBuffer src, u32 size, u32 dst_offset = 0, u32 src_offset = 0);

    Handle create_image(ColorFormat format, u32 width, u32 height);
    void destroy_image(Handle image_handle);

    Handle create_shader_internal(const Framebuffer* framebuffer, const ShaderConfig& config);
    void destroy_shader_internal(Handle shader_handle);

    void initialize_resources();
    void cleanup_resources();

    void wait_for_resources();

    void prepare_frame_resources();
    void cleanup_frame_resources();
    void submit_commands();
    void present();

    VkCommandBuffer get_command_buffer();
    RendererCommands* get_commands();

    void set_color_clear(const glm::vec4& color);
    void set_depth_clear(f32 depth_clear);
    void set_stencil_clear(u32 stencil_clear);
}

}  // namespace toki
