#pragma once

#include "engine/view.h"

class TestView : public toki::View {
public:
    virtual void on_add(const toki::Ref<toki::Renderer> renderer) override;
    virtual void on_destroy(const toki::Ref<toki::Renderer> renderer) override;
    virtual void on_render(const toki::Ref<toki::RendererApi> api) override;
    virtual void on_update(const float delta_time) override;

private:
    toki::Handle m_framebuffer_handle;
    toki::Handle m_shader_handle;
    toki::Handle m_vertex_buffer_handle;
};
