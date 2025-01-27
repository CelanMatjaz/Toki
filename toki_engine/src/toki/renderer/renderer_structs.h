#pragma once

#include "renderer/renderer_types.h"

namespace toki {

struct RenderPass {
    Handle handle;
};

struct Buffer {
    BufferType type;
    u32 offset;
    u32 size;
};

struct Texture {
    Handle handle;
};

struct Shader {
    Handle handle;
};

}  // namespace toki
