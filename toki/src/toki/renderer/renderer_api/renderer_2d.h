#pragma once

#include "toki/renderer/buffer.h"
#include "toki/renderer/camera/orthographic_camera.h"
#include "toki/renderer/render_pass.h"
#include "toki/renderer/renderer.h"
#include "toki/renderer/shader.h"

namespace Toki {

class Application;

class Renderer2D {
    friend Application;

public:
    static void init(Ref<Window> window);
    static void shutdown();

    static void begin();
    static void end();

    static void flush();

    static void drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

private:
    inline static Ref<Renderer> s_renderer;

    static void submit(Ref<RenderPass> renderPass, RendererSubmitFn submitFn);

    static void initObjects(Ref<Window> window);
};

}  // namespace Toki
