#pragma once

#include "renderer/renderer_types.h"

namespace toki {

struct RenderPass {
    Handle handle;
};

struct Buffer {
    BufferType type;
    u32 size;
    Handle handle;
};

struct Texture {
    Handle handle;
};

struct Shader {
    Handle handle;
};

}  // namespace toki
