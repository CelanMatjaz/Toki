#pragma once

#include "toki/core/core.h"

namespace Toki {

struct SamplerConfig {};

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
