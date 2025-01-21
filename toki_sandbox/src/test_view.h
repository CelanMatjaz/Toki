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

    toki::Handle m_framebufferHandle;
    toki::Handle m_shaderHandle;
    toki::Handle m_vertexBufferHandle;
    toki::Handle m_instanceBufferHandle;
    toki::Handle m_indexBufferHandle;
    toki::Handle m_uniformBufferHandle;
    toki::Handle m_textureHandle;
    toki::Handle m_textureHandle2;

    toki::Handle m_textShaderHandle;
    toki::Handle m_textVertexBufferHandle;
    toki::Handle m_textIndexBufferHandle;
    toki::Handle m_textInstanceBufferHandle;
};
