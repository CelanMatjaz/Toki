#pragma once

#include <bitset>
#include <vector>

#include "toki.h"

#define CELL_ROW_COUNT 40
#define CELL_COL_COUNT 40
#define CELL_SIZE 20

struct Cell {
    glm::vec2 position;
    glm::vec3 color;
};

class GameOfLifeView : public toki::View {
public:
    void on_add(const toki::Ref<toki::Renderer> renderer) override;
    void on_destroy(const toki::Ref<toki::Renderer> renderer) override;
    void on_update(toki::UpdateData& update_data) override;
    void on_render(const toki::Ref<toki::RendererApi> renderer) override;
    void on_event(toki::Event& event) override;

private:
    void update_cell_data();
    void update_generation();

    toki::Camera m_camera;

    toki::Handle m_framebufferHandle;
    toki::Handle m_shaderHandle;
    toki::Handle m_tileInstanceBufferHandle;
    toki::Handle m_indexBufferHandle;
    toki::Handle m_vertexBufferHandle;

    std::vector<Cell> m_cellInstanceData;
    std::bitset<CELL_COL_COUNT * CELL_ROW_COUNT> m_cells;
    uint32_t m_cellCount = 0;
    float m_updateInterval = 0.7f;  // Seconds

    bool m_isAnimationRunning = false;
    bool m_dirty = false;
    bool m_drawing = false;
    bool m_drawingState = true;
    bool m_inverted = false;
};
