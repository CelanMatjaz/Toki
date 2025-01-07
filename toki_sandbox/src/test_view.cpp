#include "test_view.h"

void test_view::on_add(const toki::ref<toki::renderer> renderer) {
    toki::render_target default_render_target{};
    default_render_target.color_format = toki::color_format::RGBA8;
    default_render_target.presentable = true;

    toki::shader_create_config shader_config{};
    shader_config.vertex_shader_path = "assets/shaders/test_shader.vert.glsl";
    shader_config.fragment_shader_path = "assets/shaders/test_shader.frag.glsl";
    shader_config.render_targets.emplace_back(default_render_target);
    _shader_handle = renderer->create_shader(shader_config);

    toki::buffer_create_config vertex_buffer_config{};
    vertex_buffer_config.size = 1024;
    vertex_buffer_config.type = toki::buffer_type::VERTEX;
    vertex_buffer_config.usage = toki::buffer_usage::DYNAMIC;
    _vertex_buffer_handle = renderer->create_buffer(vertex_buffer_config);
}

void test_view::on_destroy(const toki::ref<toki::renderer> renderer) {
    renderer->destroy_shader(_shader_handle);
    renderer->destroy_buffer(_vertex_buffer_handle);
}

void test_view::on_render(toki::ref<toki::renderer_api> api) {
    toki::begin_pass_config begin_pass_config{};
    api->begin_pass(begin_pass_config);

    api->reset_viewport();
    api->reset_scissor();

    api->bind_shader(_shader_handle);

    api->end_pass();

    api->submit();
}

void test_view::on_update(const float delta_time) {}
