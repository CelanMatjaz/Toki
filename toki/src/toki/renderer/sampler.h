#pragma once

#include "toki/core/core.h"

namespace Toki {

struct SamplerConfig {
    uint32_t setIndex = 0;
    uint32_t binding = 0;
    uint32_t arrayElement = 0;
};

class Sampler {
public:
    static Ref<Sampler> create(const SamplerConfig& config);

    Sampler() = delete;
    Sampler(const SamplerConfig& config);
    virtual ~Sampler() = default;

    uint32_t getSetIndex() const;
    uint32_t getBinding() const;
    uint32_t getArrayElementIndex() const;

private:
    SamplerConfig m_config;
};

}  // namespace Toki
