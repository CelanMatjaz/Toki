#include "test_view.h"

#include <print>

#include "core/logging.h"

void TestView::on_add(const toki::Ref<toki::Renderer> renderer) {
    toki::RenderTarget default_render_target{};
    default_render_target.loadOp = toki::RenderTargetLoadOp::CLEAR;
    default_render_target.colorFormat = toki::ColorFormat::RGBA8;
    default_render_target.presentable = true;

    toki::RenderTarget color_render_target{};
    color_render_target.colorFormat = toki::ColorFormat::RGBA8;

    toki::framebuffer_create_config framebuffer_config{};
    framebuffer_config.render_targets.emplace_back(default_render_target);
    framebuffer_config.render_targets.emplace_back(color_render_target);
    m_framebuffer_handle = renderer->create_framebuffer(framebuffer_config);

    toki::shader_create_config shader_config{};
    shader_config.vertex_shader_path = "assets/shaders/test_shader.vert.glsl";
    shader_config.fragment_shader_path = "assets/shaders/test_shader.frag.glsl";
    shader_config.framebuffer_handle = m_framebuffer_handle;
    m_shader_handle = renderer->create_shader(shader_config);

    toki::buffer_create_config vertex_buffer_config{};
    vertex_buffer_config.size = 1024;
    vertex_buffer_config.type = toki::BufferType::VERTEX;
    vertex_buffer_config.usage = toki::BufferUsage::DYNAMIC;
    m_vertex_buffer_handle = renderer->create_buffer(vertex_buffer_config);

    m_camera.set_position({ 0.0f, -1.0, 0.0f });
    m_camera.set_ortho_projection(-2, 2, 2, -2);
}

void TestView::on_destroy(const toki::Ref<toki::Renderer> renderer) {
    renderer->destroy_shader(m_shader_handle);
    renderer->destroy_buffer(m_vertex_buffer_handle);
    renderer->destroy_framebuffer(m_framebuffer_handle);
}

void TestView::on_render(toki::Ref<toki::RendererApi> api) {
    toki::BeginPassConfig begin_pass_config{};
    begin_pass_config.framebufferHandle = m_framebuffer_handle;
    begin_pass_config.viewProjectionMatrix = m_camera.get_view_projection_matrix();
    api->begin_pass(begin_pass_config);

    api->reset_viewport();
    api->reset_scissor();

    api->bind_shader(m_shader_handle);

    api->end_pass();

    api->submit();
}

void TestView::on_update(const float delta_time) {
    static float z_rotation = 0.0f;
    z_rotation += 1 * delta_time;
    m_camera.set_rotation(z_rotation);
}

void TestView::on_event(toki::Event& event) {
    using namespace toki;
    EventData data = event.get_data();

    switch (event.get_type()) {
        case EventType::KeyPress:
        case EventType::KeyRepeat:
            if (data.u16[0] == 32) {
                TK_LOG_INFO("space clicked");
            }
            break;

        default:
    }
}
