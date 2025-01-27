#pragma once

#include <functional>

#include "core/base.h"
#include "memory/allocators/basic_allocator.h"

namespace toki {

enum class ResourceType : u8 {
    Binary,
    String,
    Texture,
    FontTFF,
    Shader,
};

struct Resource {
    ResourceType type;
    // BasicRef<byte> data;
};

}  // namespace toki
