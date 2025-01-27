#include "test_view.h"

#include "cube_data.h"
#include "renderer/renderer_commands.h"

void TestView::on_add() {
    toki::Renderer& renderer = get_renderer();

    vertex_buffer = renderer.create_buffer(toki::BufferType::Vertex, std::size(cube_vertices));
    index_buffer = renderer.create_buffer(toki::BufferType::Index, std::size(cube_indices));
    toki::RenderPass render_pass;
    shader = renderer.create_shader(
        render_pass, toki::configs::load_shader_config("configs/test_shader_config.yaml"));
}

void TestView::on_destroy() {}

void TestView::on_render() {
    toki::Renderer& renderer = get_renderer();
    renderer.submit([&](toki::RendererCommands& cmd) {
        cmd.begin_pass({ { 0, 0 }, { 600, 600 } });
        cmd.reset_viewport();
        cmd.reset_scissor();
        cmd.bind_shader(shader);
        cmd.bind_buffer(vertex_buffer);
        cmd.bind_buffer(index_buffer);
        cmd.draw_indexed(36);
        cmd.end_pass();
    });
}

void TestView::on_update(float delta_time) {}

void TestView::on_event(toki::Event& event) {}
