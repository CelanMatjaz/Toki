#include "resource_system.h"

#include <filesystem>
#include <unordered_map>

#include "toki/resources/resource.h"
#include "toki/resources/resource_utils.h"

namespace Toki {

struct ResourceSystemState {
    ResourceSystemConfig config;
    std::unordered_map<std::filesystem::path, Resource> resourceMap;
};

static ResourceSystemState* state;

void ResourceSystem::initialize(const ResourceSystemConfig& config) {
    state = new ResourceSystemState();
    state->config = config;

    ResourceUtils::ensureDirectory(config.resourceDirectory);

    findResources();
}

void ResourceSystem::shutdown() {
    delete state;
}

void ResourceSystem::setRootDirectory(std::filesystem::path path) {
    state->config.resourceDirectory = path;
}

void ResourceSystem::findResources() {
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(state->config.resourceDirectory)) {
        std::filesystem::path dirEntryPath = dirEntry.path();

        if (std::filesystem::is_directory(dirEntryPath)) {
            continue;
        }

        if (state->resourceMap.contains(dirEntryPath) && state->resourceMap[dirEntryPath].checkForNewWrite()) {
            state->resourceMap[dirEntryPath].update();
            continue;
        }

        state->resourceMap.emplace(dirEntryPath, Resource{ ResourceType::Unknown, dirEntryPath });
    }
}

std::vector<WrappedRef<Resource>> ResourceSystem::getResourcesAtPath(std::filesystem::path path) {
    std::vector<WrappedRef<Resource>> results;

    std::filesystem::path matchPath = path.is_relative() ? state->config.resourceDirectory / path.relative_path() : path;

    for (const auto& [resourcePath, resource] : state->resourceMap) {
        if (resourcePath.parent_path() == matchPath) {
            results.emplace_back(createWrappedRef<Resource>(&resource));
        }
    }

    return results;
}

}  // namespace Toki
