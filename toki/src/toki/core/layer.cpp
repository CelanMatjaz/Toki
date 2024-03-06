#include "toki/core/layer.h"

namespace Toki {

void Layer::submit(Ref<RenderPass> renderpass, RendererSubmitFn submitFn) {
    s_renderer->submit(renderpass, submitFn);
}

}  // namespace Toki
