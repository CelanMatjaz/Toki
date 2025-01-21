#pragma once

#include <vulkan/vulkan.h>

#include "core/macros.h"
#include "engine/window.h"
#include "renderer/renderer.h"

namespace toki {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer() = delete;
    VulkanRenderer(const Config& config);
    ~VulkanRenderer();

    DELETE_COPY(VulkanRenderer);
    DELETE_MOVE(VulkanRenderer);

public:
    virtual Handle create_shader(const ShaderCreateConfig& config) override;
    virtual void destroy_shader(Handle shader_handle) override;

    virtual Handle create_buffer(const BufferCreateConfig& config) override;
    virtual void destroy_buffer(Handle buffer_handle) override;

    virtual Handle create_texture(const TextureCreateConfig& config) override;
    virtual void destroy_texture(Handle texture_handle) override;

    virtual Handle create_framebuffer(const FramebufferCreateConfig& config) override;
    virtual void destroy_framebuffer(Handle framebuffer_handle) override;

    virtual void set_buffer_data(Handle buffer_handle, u32 size, void* data) override;
    virtual void set_texture_data(Handle texture_handle, u32 size, void* data) override;

public:
    virtual void begin_pass(const BeginPassConfig& config) override;
    virtual void end_pass() override;
    virtual void submit() override;

    virtual void bind_shader(Handle handle) override;
    virtual void bind_vertex_buffer(Handle handle) override;
    virtual void bind_vertex_buffer(Handle handle, u32 binding) override;
    virtual void bind_vertex_buffers(const BindVertexBuffersConfig& config) override;
    virtual void bind_index_buffer(Handle handle) override;
    virtual void push_constant(Handle shader_handle, u32 size, void* data) override;

    virtual void update_sets(Handle shader_handle) override;
    virtual void bind_descriptor_sets(Handle shader_handle) override;
    virtual void reset_descriptor_sets(Handle shader_handle) override;
    virtual void write_buffer(Handle shader_handle, Handle buffer_handle, u32 set, u32 binding) override;
    virtual void write_texture(Handle shader_handle, Handle texture_handle, u32 set, u32 binding) override;

    virtual void reset_viewport() override;
    virtual void reset_scissor() override;

    virtual void draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) override;
    virtual void draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) override;

private:
    void add_window(Window* window);

    virtual b8 begin_frame() override;
    virtual void end_frame() override;
    virtual void present() override;

    virtual void wait_for_resources() override;

private:
    void create_instance();
    void create_device(Window* window);
    void create_command_pools();
    void create_descriptor_pools();
    void create_default_resources();

    void cleanup_default_resources();
};

}  // namespace toki
