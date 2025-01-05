#include "test_view.h"

void TestView::on_add(const toki::Ref<toki::Renderer> renderer) {
    toki::Shader::Config shader_config{};
    shader_config.vertex_shader_path = "assets/shaders/test_shader.vert.glsl";
    shader_config.fragment_shader_path = "assets/shaders/test_shader.frag.glsl";
    m_shader = renderer->create_shader(shader_config);
}

void TestView::on_destroy() {}

void TestView::on_render(toki::Ref<toki::RendererApi> api) {
    api->reset_viewport();
    api->reset_scissor();
}

void TestView::on_update(const float delta_time) {}
