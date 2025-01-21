#pragma once

#include "engine/window.h"
#include "renderer/configs.h"

namespace toki {

class Engine;
struct RendererContext;

class Renderer {
    friend Engine;

public:
    struct Config {
        Window* initialWindow;
    };

protected:
    static Renderer* create(const Config& config);

    Renderer() = delete;
    Renderer(const Config& config);

public:
    virtual ~Renderer() = default;

private:
    DELETE_COPY(Renderer);
    DELETE_MOVE(Renderer);

public:
    virtual Handle create_shader(const ShaderCreateConfig& config) = 0;
    virtual void destroy_shader(Handle shader_handle) = 0;

    virtual Handle create_buffer(const BufferCreateConfig& config) = 0;
    virtual void destroy_buffer(Handle buffer_handle) = 0;

    virtual Handle create_texture(const TextureCreateConfig& config) = 0;
    virtual void destroy_texture(Handle texture_handle) = 0;

    virtual Handle create_framebuffer(const FramebufferCreateConfig& config) = 0;
    virtual void destroy_framebuffer(Handle framebuffer_handle) = 0;

    virtual void set_buffer_data(Handle buffer_handle, u32 size, void* data) = 0;
    virtual void set_texture_data(Handle texture_handle, u32 size, void* data) = 0;

public:
    virtual void begin_pass(const BeginPassConfig& config) = 0;
    virtual void end_pass() = 0;
    virtual void submit() = 0;

    virtual void bind_shader(Handle handle) = 0;
    virtual void bind_vertex_buffer(Handle handle) = 0;
    virtual void bind_vertex_buffer(Handle handle, u32 binding) = 0;
    virtual void bind_vertex_buffers(const BindVertexBuffersConfig& config) = 0;
    virtual void bind_index_buffer(Handle handle) = 0;
    virtual void push_constant(Handle shader_handle, u32 size, void* data) = 0;

    virtual void update_sets(Handle shader_handle) = 0;
    virtual void reset_descriptor_sets(Handle shader_handle) = 0;
    virtual void bind_descriptor_sets(Handle shader_handle) = 0;
    virtual void write_buffer(Handle shader_handle, Handle buffer_handle, u32 set, u32 binding) = 0;
    virtual void write_texture(Handle shader_handle, Handle texture_handle, u32 set, u32 binding) = 0;

    virtual void reset_viewport() = 0;
    virtual void reset_scissor() = 0;

    virtual void draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) = 0;
    virtual void draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) = 0;

protected:
    virtual b8 begin_frame() = 0;
    virtual void end_frame() = 0;
    virtual void present() = 0;

    virtual void wait_for_resources() = 0;

    void add_window(Window* window);
    RendererContext* m_context;
};

}  // namespace toki
