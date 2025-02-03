#include "test_view.h"

#include "cube_data.h"
#include "renderer/renderer_commands.h"

void TestView::on_add() {
    toki::Renderer& renderer = get_renderer();

    {
        toki::FramebufferConfig framebuffer_config{};
        framebuffer_config.has_present_attachment = true;
        framebuffer_config.has_depth_attachment = true;
        framebuffer_config.attachment_dimensions = { 600, 600 };
        m_framebuffer = renderer.create_framebuffer(framebuffer_config);
    }

    {
        toki::BufferConfig vertex_buffer_config{};
        vertex_buffer_config.type = toki::BufferType::Vertex;
        vertex_buffer_config.size = sizeof(cube_vertices);
        m_vertexBuffer = renderer.create_buffer(vertex_buffer_config);
        renderer.set_bufffer_data(&m_vertexBuffer, sizeof(cube_vertices), cube_vertices);
    }

    {
        toki::BufferConfig index_buffer_config{};
        index_buffer_config.type = toki::BufferType::Index;
        index_buffer_config.size = sizeof(cube_indices);
        m_indexBuffer = renderer.create_buffer(index_buffer_config);
        renderer.set_bufffer_data(&m_indexBuffer, sizeof(cube_indices), cube_indices);
    }

    {
        toki::ShaderConfig shader_config{};
        shader_config.config_path = "configs/test_shader_config.yaml";
        m_shader = renderer.create_shader(&m_framebuffer, shader_config);
    }
}

void TestView::on_destroy() {
    toki::Renderer& renderer = get_renderer();
    renderer.destroy_buffer(&m_vertexBuffer);
    renderer.destroy_buffer(&m_indexBuffer);
}

void TestView::on_render() {
    toki::Renderer& renderer = get_renderer();
    renderer.submit([&](toki::RendererCommands& cmd) {
        cmd.begin_rendering(&m_framebuffer, { { 0, 0 }, { 600, 600 } });
        cmd.reset_viewport();
        cmd.reset_scissor();
        cmd.bind_shader(m_shader);
        cmd.bind_buffer(m_vertexBuffer);
        cmd.bind_buffer(m_indexBuffer);
        cmd.draw_indexed(36);
        cmd.end_rendering();
    });
}

void TestView::on_update([[maybe_unused]] float delta_time) {}

void TestView::on_event([[maybe_unused]] toki::Event& event) {}
