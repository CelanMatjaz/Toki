#pragma once

#include "renderer/renderer_api.h"
#include "renderer/vulkan/vulkan_context.h"

namespace toki {

class VulkanRendererApi : public RendererApi {
public:
    VulkanRendererApi(Ref<RendererContext> context);
    ~VulkanRendererApi();

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

    virtual void reset_viewport() override;
    virtual void reset_scissor() override;

    virtual void draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) override;
    virtual void draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) override;

private:
    Ref<RendererContext> m_context{};

    b8 m_isPassStarted = false;

    void fix_render_area(Rect2D& render_area);
};

}  // namespace toki
