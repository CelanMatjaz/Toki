#pragma once

#include <cstdint>

#define RESOURCE_TYPE(str) (uint32_t) str[0] | (uint32_t) str[1] << 8 | (uint32_t) str[2] << 16 | (uint32_t) str[3] << 24;

struct Metadata {
    uint32_t resourceType;
    uint32_t serializerVersion;
    uint64_t timeSaved;
};