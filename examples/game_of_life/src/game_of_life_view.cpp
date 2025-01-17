#include "game_of_life_view.h"

void GameOfLifeView::on_add(const toki::Ref<toki::Renderer> renderer) {
    m_cellInstanceData.resize(CELL_COL_COUNT * CELL_ROW_COUNT);
    m_inverted = true;

    {
        toki::RenderTarget default_render_target{};
        default_render_target.colorFormat = toki::ColorFormat::RGBA8;
        default_render_target.presentable = true;

        toki::FramebufferCreateConfig framebuffer_config{};
        framebuffer_config.render_targets.emplace_back(default_render_target);
        m_framebufferHandle = renderer->create_framebuffer(framebuffer_config);
    }

    {
        toki::ShaderCreateConfig shader_config{};
        shader_config.config = toki::configs::load_shader_config("configs/game_of_life_shader_config.yaml");
        shader_config.framebuffer_handle = m_framebufferHandle;
        m_shaderHandle = renderer->create_shader(shader_config);
    }

    {
        uint32_t quad_indices[] = { 0, 1, 2, 2, 1, 3 };

        toki::BufferCreateConfig index_buffer_config{};
        index_buffer_config.size = sizeof(quad_indices);
        index_buffer_config.type = toki::BufferType::INDEX;
        index_buffer_config.usage = toki::BufferUsage::DYNAMIC;
        m_indexBufferHandle = renderer->create_buffer(index_buffer_config);
        renderer->set_buffer_data(m_indexBufferHandle, sizeof(quad_indices), quad_indices);
    }

    {
        glm::vec2 positions[] = { glm::vec2(0.0, 0.0), glm::vec2(0.0, 1.0), glm::vec2(1.0, 0.0), glm::vec2(1.0, 1.0) };

        toki::BufferCreateConfig vertex_buffer_config{};
        vertex_buffer_config.size = sizeof(positions);
        vertex_buffer_config.type = toki::BufferType::VERTEX;
        vertex_buffer_config.usage = toki::BufferUsage::DYNAMIC;
        m_vertexBufferHandle = renderer->create_buffer(vertex_buffer_config);
        renderer->set_buffer_data(m_vertexBufferHandle, sizeof(positions), positions);
    }

    {
        toki::BufferCreateConfig instance_buffer_config{};
        instance_buffer_config.size = sizeof(Cell) * CELL_ROW_COUNT * CELL_COL_COUNT;
        instance_buffer_config.type = toki::BufferType::VERTEX;
        instance_buffer_config.usage = toki::BufferUsage::DYNAMIC;
        m_tileInstanceBufferHandle = renderer->create_buffer(instance_buffer_config);
    }

    m_camera.set_ortho_projection(0.0f, CELL_COL_COUNT, 0.0f, -CELL_ROW_COUNT);

    m_cellInstanceData.emplace_back(Cell{ glm::vec2{ 0.0f, 0.0f }, glm::vec3{ 1.0f, 1.0f, 1.0f } });
    renderer->set_buffer_data(m_tileInstanceBufferHandle, sizeof(Cell), m_cellInstanceData.data());
}

void GameOfLifeView::on_destroy(const toki::Ref<toki::Renderer> renderer) {
    renderer->destroy_buffer(m_tileInstanceBufferHandle);
    renderer->destroy_framebuffer(m_framebufferHandle);
    renderer->destroy_shader(m_shaderHandle);
}

void GameOfLifeView::on_update(toki::UpdateData& update_data) {
    if (!m_isAnimationRunning && !m_dirty) {
        return;
    }

    static float time_since_last_update = 0.0f;
    if (time_since_last_update >= m_updateInterval && m_isAnimationRunning) {
        time_since_last_update = 0;
        update_generation();
    }

    if (m_dirty) {
        update_cell_data();
        if (m_cellCount > 0) {
            update_data.renderer->set_buffer_data(m_tileInstanceBufferHandle, sizeof(Cell) * m_cellCount, m_cellInstanceData.data());
        }
    }

    time_since_last_update += update_data.delta_time;
}

struct {
    glm::mat4 mvp;
    float screen_width = CELL_COL_COUNT;
    float screen_height = CELL_ROW_COUNT;
} push;

void GameOfLifeView::on_render(const toki::Ref<toki::RendererApi> api) {
    toki::BeginPassConfig begin_pass_config{};
    begin_pass_config.framebufferHandle = m_framebufferHandle;
    api->begin_pass(begin_pass_config);

    api->reset_viewport();
    api->reset_scissor();

    if (m_cellCount > 0) {
        api->bind_shader(m_shaderHandle);
        toki::BindVertexBuffersConfig vertex_buffers_config{};
        vertex_buffers_config.handles = { m_vertexBufferHandle, m_tileInstanceBufferHandle };
        vertex_buffers_config.first_binding = 0;
        api->bind_vertex_buffers(vertex_buffers_config);
        api->bind_index_buffer(m_indexBufferHandle);

        push.mvp = m_camera.get_view_projection_matrix();
        api->push_constant(m_shaderHandle, sizeof(push), &push);

        api->draw_indexed(6, m_cellCount, 0, 0, 0);
    }

    api->end_pass();

    api->submit();
}

uint32_t get_index(glm::ivec2 position) {
    return (position.x / CELL_SIZE) + (position.y / CELL_SIZE) * CELL_ROW_COUNT;
}

void GameOfLifeView::on_event(toki::Event& event) {
    using namespace toki;

    auto data = event.get_data();
    switch (event.get_type()) {
        case EventType::MousePress: {
            if (m_isAnimationRunning) {
                break;
            }

            m_drawing = true;
            uint32_t index = get_index(event.as<toki::MousePressEvent>().get_position());
            m_drawingState = !m_cells[index];
            m_cells[index].flip();

            break;
        }

        case EventType::MouseRelease: {
            m_drawing = false;
            break;
        }

        case EventType::MouseMove: {
            if (!m_drawing) {
                break;
            }

            uint32_t index = get_index(event.as<toki::MouseMoveEvent>().get_position());
            if (index >= m_cells.size()) {
                break;
            }

            m_cells[index] = m_drawingState;
            m_dirty = true;
            update_cell_data();
            break;
        }

        case EventType::KeyPress:
            switch (data.u16[0]) {
                case (uint32_t) toki::KeyCode::KEY_SPACE:
                    m_isAnimationRunning = !m_isAnimationRunning;
                    break;
                case (uint32_t) toki::KeyCode::KEY_R:
                    m_isAnimationRunning = false;
                    m_cells = 0;
                    break;
                case (uint32_t) toki::KeyCode::KEY_I:
                    m_inverted = !m_inverted;
                    m_dirty = true;
                    break;
            }
            break;

        default: {
        }
    }
}

void GameOfLifeView::update_cell_data() {
    m_cellCount = 0;
    for (uint32_t i = 0; i < m_cells.size(); i++) {
        if (m_cells[i] != m_inverted) {
            continue;
        }

        m_cellInstanceData[m_cellCount++] = { glm::vec2{ i % CELL_COL_COUNT, i / CELL_ROW_COUNT }, glm::vec3{ 1.0 } };
    }
}

void GameOfLifeView::update_generation() {
    auto copy = m_cells;

    constexpr int neighbors[8][2] = { { -1, -1 }, { -1, 0 }, { -1, 1 }, { 0, -1 }, { 0, 1 }, { 1, -1 }, { 1, 0 }, { 1, 1 } };
    for (int32_t i = 0; i < m_cells.size(); i++) {
        uint32_t neightbor_count = 0;

        for (const auto& [row_offset, col_offset] : neighbors) {
            uint32_t row = i / CELL_ROW_COUNT + row_offset;
            uint32_t col = i % CELL_ROW_COUNT + col_offset;

            if (row < 0 || row >= CELL_ROW_COUNT || col < 0 || col >= CELL_COL_COUNT) {
                continue;
            }

            int index = row * CELL_ROW_COUNT + col;
            if (m_cells[index]) {
                neightbor_count++;
            }
        }

        if (m_cells[i] && (neightbor_count <= 1 || neightbor_count >= 4)) {
            copy[i] = false;
        } else if (!m_cells[i] && neightbor_count == 3) {
            copy[i] = true;
        }
    }

    m_cells = copy;
    m_dirty = true;
}
