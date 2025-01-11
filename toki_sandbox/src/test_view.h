#pragma once

#include "toki.h"

class TestView : public toki::View {
public:
    virtual void on_add(const toki::Ref<toki::Renderer> renderer) override;
    virtual void on_destroy(const toki::Ref<toki::Renderer> renderer) override;
    virtual void on_render(const toki::Ref<toki::RendererApi> api) override;
    virtual void on_update(toki::UpdateData& update_data) override;
    virtual void on_event(toki::Event& event) override;

private:
    toki::Camera m_camera;

    toki::Handle m_framebufferHandle;
    toki::Handle m_shaderHandle;
    toki::Handle m_vertexBufferHandle;
    toki::Handle m_instanceBufferHandle;
    toki::Handle m_indexBufferHandle;
};
