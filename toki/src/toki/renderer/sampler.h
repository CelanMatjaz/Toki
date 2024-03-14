#pragma once

#include "toki/core/core.h"

namespace Toki {

enum class TextureRepeat {
    Repeat,
    Mirrored,
    ClampEdge,
    ClampBorder
};

enum class TextureFilter {
    Nearest,
    Linear
};

struct SamplerConfig {
    TextureRepeat repeatU : 2 = TextureRepeat::Repeat;
    TextureRepeat repeatV : 2 = TextureRepeat::Repeat;
    TextureRepeat repeatW : 2 = TextureRepeat::Repeat;
    TextureFilter magFilter : 2 = TextureFilter::Nearest;
    TextureFilter minFilter : 2 = TextureFilter::Nearest;
};

class Sampler {
public:
    static Ref<Sampler> create(const SamplerConfig& config);

    Sampler() = delete;
    Sampler(const SamplerConfig& config);
    virtual ~Sampler() = default;

private:
    SamplerConfig m_config;
};

}  // namespace Toki
