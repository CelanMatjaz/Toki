#pragma once

#include "core/base.h"
#include "renderer/renderer_types.h"

namespace toki {

enum class TextureType {
    Single2D,
    Array2D,
};

struct Texture {
    Handle handle;
    u32 width;
    u32 height;
    ColorFormat format;
    u32 array_size = 1;
};

}  // namespace toki
