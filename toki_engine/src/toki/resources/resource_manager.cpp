#include "resource_manager.h"

namespace toki {

ResourceManager::ResourceManager() {}

Resource ResourceManager::load_resource(ResourceType type, std::string_view path) {
    Resource resource{ type };

    // switch (type) {
    //     case ResourceType::Binary:
    //     case ResourceType::String:
    //     case ResourceType::Texture:
    //     case ResourceType::FontTFF:
    //     case ResourceType::Shader:
    // }

    return resource;
}

void ResourceManager::unload_resource(Resource& resource) {
    // resource.data.reset();
}

}  // namespace toki
