#pragma once

#include "toki.h"

class TestView : public toki::View {
public:
    virtual void on_add() override;
    virtual void on_destroy() override;
    virtual void on_render() override;
    virtual void on_update(float delta_time) override;
    virtual void on_event(toki::Event& event) override;

private:
    toki::Camera m_camera;

    toki::Buffer vertex_buffer;
    toki::Buffer index_buffer;
    toki::Shader shader;
};
