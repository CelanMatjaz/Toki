#pragma once

#include <vector>

#include "core/base.h"

namespace toki {

enum class ShaderUniformType : u8 {
    None,
    Float3,
    Float4,
    Struct,
    Texture2D,
    Sampler
};

struct ShaderUniform {
    ShaderUniformType type;
    u8 location;
    u16 size;
};

struct ShaderConstant {
    u16 size;
    u16 offset;
};

struct ShaderUniformSet {
    u8 index;
    std::vector<ShaderUniform> uniforms;
};

}  // namespace toki
