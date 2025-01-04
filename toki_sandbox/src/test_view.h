#pragma once

#include "engine/view.h"

class TestView : public toki::View {
public:
    virtual void on_add(const toki::Ref<toki::Renderer> renderer) override;
    virtual void on_destroy() override;
    virtual void on_render(const toki::Ref<toki::RendererApi> api) override;
    virtual void on_update(const float deltaTime) override;

private:
    toki::Ref<toki::Shader> m_shader;
};
