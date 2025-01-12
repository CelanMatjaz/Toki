#include "test_view.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include "core/logging.h"
#include "cube_data.h"
#include "resources/configs/shader_config_loader.h"

float rotation = 0.0f;
bool holding = false;

void TestView::on_add(const toki::Ref<toki::Renderer> renderer) {
    {
        toki::RenderTarget default_render_target{};
        default_render_target.colorFormat = toki::ColorFormat::RGBA8;
        default_render_target.presentable = true;

        toki::RenderTarget color_render_target{};
        color_render_target.colorFormat = toki::ColorFormat::RGBA8;

        toki::RenderTarget depth_render_target{};
        depth_render_target.colorFormat = toki::ColorFormat::DEPTH;
        depth_render_target.loadOp = toki::RenderTargetLoadOp::CLEAR;
        depth_render_target.storeOp = toki::RenderTargetStoreOp::STORE;

        toki::FramebufferCreateConfig framebuffer_config{};
        framebuffer_config.render_targets.emplace_back(default_render_target);
        framebuffer_config.render_targets.emplace_back(color_render_target);
        framebuffer_config.render_targets.emplace_back(depth_render_target);
        m_framebufferHandle = renderer->create_framebuffer(framebuffer_config);
    }

    {
        toki::ShaderCreateConfig shader_config{};
        shader_config.config = toki::configs::load_shader_config("configs/test_shader_config.yaml");
        shader_config.framebuffer_handle = m_framebufferHandle;
        m_shaderHandle = renderer->create_shader(shader_config);
    }

    {
        toki::BufferCreateConfig vertex_buffer_config{};
        vertex_buffer_config.size = sizeof(cube_vertices);
        vertex_buffer_config.type = toki::BufferType::VERTEX;
        vertex_buffer_config.usage = toki::BufferUsage::DYNAMIC;
        m_vertexBufferHandle = renderer->create_buffer(vertex_buffer_config);
        renderer->set_buffer_data(m_vertexBufferHandle, sizeof(cube_vertices), cube_vertices);
    }

    {
        toki::BufferCreateConfig index_buffer_config{};
        index_buffer_config.size = sizeof(cube_indices);
        index_buffer_config.type = toki::BufferType::INDEX;
        index_buffer_config.usage = toki::BufferUsage::DYNAMIC;
        m_indexBufferHandle = renderer->create_buffer(index_buffer_config);
        renderer->set_buffer_data(m_indexBufferHandle, sizeof(cube_indices), cube_indices);
    }

    {
        struct InstanceData {
            glm::vec3 pos;
            glm::vec3 color;
        };

        InstanceData instances[] = { { glm::vec3{ -0.0f, -0.0f, 0.0f }, glm::vec3{ 0.0f, 0.4f, 0.6f } },  //
                                     { glm::vec3{ -0.5f, -0.5f, 0.5f }, glm::vec3{ 0.6f, 0.4f, 0.7f } } };

        toki::BufferCreateConfig instance_buffer_config{};
        instance_buffer_config.size = sizeof(instances);
        instance_buffer_config.type = toki::BufferType::VERTEX;
        instance_buffer_config.usage = toki::BufferUsage::DYNAMIC;
        m_instanceBufferHandle = renderer->create_buffer(instance_buffer_config);
        renderer->set_buffer_data(m_instanceBufferHandle, sizeof(instances), instances);
    }

    {
        toki::BufferCreateConfig uniform_buffer_config{};
        uniform_buffer_config.size = sizeof(glm::mat4);
        uniform_buffer_config.type = toki::BufferType::UNIFORM;
        uniform_buffer_config.usage = toki::BufferUsage::DYNAMIC;
        m_uniformBufferHandle = renderer->create_buffer(uniform_buffer_config);
    }

    m_camera.set_position({ 0.0f, 0.0f, -5.0f });
    m_camera.set_perspective_projection(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
}

void TestView::on_destroy(const toki::Ref<toki::Renderer> renderer) {
    renderer->destroy_shader(m_shaderHandle);
    renderer->destroy_framebuffer(m_framebufferHandle);
    renderer->destroy_buffer(m_indexBufferHandle);
    renderer->destroy_buffer(m_vertexBufferHandle);
    renderer->destroy_buffer(m_uniformBufferHandle);
}

glm::mat4 model;

void TestView::on_render(toki::Ref<toki::RendererApi> api) {
    toki::BeginPassConfig begin_pass_config{};
    begin_pass_config.framebufferHandle = m_framebufferHandle;
    begin_pass_config.viewProjectionMatrix = m_camera.get_view_projection_matrix() * model;
    api->begin_pass(begin_pass_config);

    api->reset_viewport();
    api->reset_scissor();

    api->bind_shader(m_shaderHandle);

    toki::BindVertexBuffersConfig bind_vertex_buffers_config{};
    bind_vertex_buffers_config.handles = { m_vertexBufferHandle, m_instanceBufferHandle };
    api->bind_vertex_buffers(bind_vertex_buffers_config);
    api->bind_index_buffer(m_indexBufferHandle);

    api->draw_indexed(36, 2, 0, 0, 0);

    api->end_pass();

    api->submit();
}

void TestView::on_update(toki::UpdateData& update_data) {
    rotation += 0.0001;

    model = glm::eulerAngleXYZ(rotation * 1, rotation * 2, rotation * 3) * update_data.delta_time;

    using namespace toki;
    if (update_data.input->is_key_down(KeyCode::KEY_W)) {
        m_camera.add_position(glm::vec3{ 0.0f, 0.0f, +1.0f } * update_data.delta_time);
    } else if (update_data.input->is_key_down(KeyCode::KEY_S)) {
        m_camera.add_position(glm::vec3{ 0.0f, 0.0f, -1.0f } * update_data.delta_time);
    }

    if (update_data.input->is_key_down(KeyCode::KEY_A)) {
        m_camera.add_position(glm::vec3{ +1.0f, 0.0f, 0.0f } * update_data.delta_time);
    } else if (update_data.input->is_key_down(KeyCode::KEY_D)) {
        m_camera.add_position(glm::vec3{ -1.0f, 0.0f, 0.0f } * update_data.delta_time);
    }
}

void TestView::on_event(toki::Event& event) {
    using toki::EventType, toki::EventData;
    EventData data = event.get_data();
    static int32_t prev_x = 0;

    switch (event.get_type()) {
        case EventType::MouseClick: {
            prev_x = data.i16[2];
            holding = true;
            break;
        }
        case EventType::MouseRelease: {
            holding = false;
            prev_x = 0;

            break;
        }

        case EventType::MouseMove: {
            if (!holding) {
                break;
            }

            static float rotation = 0.0f;
            rotation += (prev_x - data.i32[0]) * 0.001;
            prev_x = data.i32[0];
            m_camera.set_rotation(rotation);

            break;
        }

        case EventType::WindowResize: {
            static float width, height;
            width = data.u32[0];
            height = data.u32[1];

            m_camera.set_perspective_projection(glm::radians(90.0f), width / height, 0.01f, 100.0f);
            break;
        }

        case EventType::KeyPress:
        case EventType::KeyRepeat:
            if (data.u16[0] == 32) {
                TK_LOG_INFO("space clicked");
            }
            break;

        case EventType::MouseScroll: {
            float a = 0;
            if (data.f32[1] > 0) {
                a += 0.1f;
            } else if (data.f32[1] < 0) {
                a -= 0.1f;
            }

            m_camera.add_position(glm::vec3{ 0.0f, 0.0f, a });
            break;
        }

        default: {
        }
    }
}
