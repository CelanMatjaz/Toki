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
    m_framebufferHandle = renderer->create_framebuffer(framebuffer_config);

    toki::shader_create_config shader_config{};
    shader_config.vertex_shader_path = "assets/shaders/test_shader.vert.glsl";
    shader_config.fragment_shader_path = "assets/shaders/test_shader.frag.glsl";
    shader_config.framebuffer_handle = m_framebufferHandle;
    m_shaderHandle = renderer->create_shader(shader_config);

    float vertices[] = {
        // -0.5f, -0.5f, 0.0f,  //
        // +0.5f, -0.5f, 0.0f,  //
        // +0.0f, +0.5f, 0.0f,  //
        //
        //
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,

    };

    toki::buffer_create_config vertex_buffer_config{};
    vertex_buffer_config.size = sizeof(vertices);
    vertex_buffer_config.type = toki::BufferType::VERTEX;
    vertex_buffer_config.usage = toki::BufferUsage::DYNAMIC;
    m_vertexBufferHandle = renderer->create_buffer(vertex_buffer_config);

    renderer->set_buffer_data(m_vertexBufferHandle, sizeof(vertices), vertices);

    uint32_t indices[] = { 0, 1, 2, 0, 3, 1 };

    toki::buffer_create_config index_buffer_config{};
    index_buffer_config.size = sizeof(indices);
    index_buffer_config.type = toki::BufferType::INDEX;
    index_buffer_config.usage = toki::BufferUsage::DYNAMIC;
    m_indexBufferHandle = renderer->create_buffer(index_buffer_config);

    renderer->set_buffer_data(m_indexBufferHandle, sizeof(indices), indices);

    m_camera.set_position({ 0.0f, -1.0, 0.0f });
    m_camera.set_ortho_projection(-2, 2, 2, -2);
}

void TestView::on_destroy(const toki::Ref<toki::Renderer> renderer) {
    renderer->destroy_shader(m_shaderHandle);
    renderer->destroy_buffer(m_vertexBufferHandle);
    renderer->destroy_framebuffer(m_framebufferHandle);
}

void TestView::on_render(toki::Ref<toki::RendererApi> api) {
    toki::BeginPassConfig begin_pass_config{};
    begin_pass_config.framebufferHandle = m_framebufferHandle;
    begin_pass_config.viewProjectionMatrix = m_camera.get_view_projection_matrix();
    api->begin_pass(begin_pass_config);

    api->reset_viewport();
    api->reset_scissor();

    api->bind_shader(m_shaderHandle);
    api->bind_vertex_buffer(m_vertexBufferHandle);
    api->bind_index_buffer(m_indexBufferHandle);

    api->draw_indexed(6, 1, 0, 0, 0);

    api->end_pass();

    api->submit();
}

void TestView::on_update(const toki::Ref<toki::Renderer> renderer, const float delta_time) {
    static float z_rotation = 0.0f;
    z_rotation += 1 * delta_time;
    m_camera.set_rotation(z_rotation);
}

void TestView::on_event(toki::Event& event) {
    using toki::EventType, toki::EventData;
    EventData data = event.get_data();

    switch (event.get_type()) {
        case EventType::KeyPress:
        case EventType::KeyRepeat:
            if (data.u16[0] == 32) {
                TK_LOG_INFO("space clicked");
            }
            break;

        case EventType::MouseScroll: {
            static float a = 2.0f;
            if (data.f32[1] > 0) {
                a -= 0.1f;
            }

            else if (data.f32[1] < 0) {
                a += 0.1f;
            }
            m_camera.set_ortho_projection(-a, a, a, -a);
        }

        default:
    }
}
