#pragma once

#include "engine/view.h"

class test_view : public toki::view {
public:
    virtual void on_add(const toki::ref<toki::renderer> renderer) override;
    virtual void on_destroy(const toki::ref<toki::renderer> renderer) override;
    virtual void on_render(const toki::ref<toki::renderer_api> api) override;
    virtual void on_update(const float delta_time) override;

private:
    toki::handle _framebuffer_handle;
    toki::handle _shader_handle;
    toki::handle _vertex_buffer_handle;
};
