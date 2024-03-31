#pragma once

#include <filesystem>

#include "toki/core/core.h"
#include "toki/resources/resource.h"

namespace Toki {

struct ResourceSystemConfig {
    std::filesystem::path resourceDirectory;
};

class Application;

class ResourceSystem {
    friend Application;

public:
    static void setRootDirectory(std::filesystem::path path);
    static void findResources();
    static std::vector<WrappedRef<Resource>> getResourcesAtPath(std::filesystem::path path);

private:
    static void initialize(const ResourceSystemConfig& config);
    static void shutdown();
};

}  // namespace Toki
