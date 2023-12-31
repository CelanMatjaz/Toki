#pragma once

namespace Toki {

enum class ShaderStage {
    None,
    Vertex,
    Fragment
};

enum class ShaderType {
    None,
    Graphics,
    Compute
};

enum class Format {
    None,
    R,
    RG,
    RGB,
    RGBA,
    Depth,
    DepthStencil
};

}  // namespace Toki
