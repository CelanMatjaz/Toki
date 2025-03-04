#pragma once

#include <bitset>
#include <vector>

#include "toki.h"

#define CELL_ROW_COUNT 80
#define CELL_COL_COUNT 80
#define CELL_SIZE 10

struct Cell {
    glm::vec2 position;
    glm::vec3 color;
};

class GameOfLifeView : public toki::View {
public:
    void on_add() override;
    void on_destroy() override;
    void on_update(float delta_time) override;
    void on_render() override;
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
