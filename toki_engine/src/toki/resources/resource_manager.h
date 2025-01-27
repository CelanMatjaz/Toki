#pragma once

#include <string_view>

#include "memory/allocators/basic_allocator.h"
#include "resources.h"

namespace toki {

class ResourceManager {
public:
    struct Config {
        u32 max_resources;
    };

public:
    ResourceManager();

    Resource load_resource(ResourceType type, std::string_view path);
    void unload_resource(Resource& resource);

private:
    BasicAllocator m_allocator;
};

}  // namespace toki
