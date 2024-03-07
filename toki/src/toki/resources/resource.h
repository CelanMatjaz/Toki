#pragma once

#include <filesystem>

namespace Toki {

enum class ResourceType {
    File
};

class Resource {
public:
    Resource(ResourceType type, std::filesystem::path path);
    ~Resource() = default;

    bool checkForNewWrite();

    ResourceType getType();
    std::filesystem::path getPath();
    std::filesystem::file_time_type getLastWriteTime();

private:
    ResourceType m_type;
    std::filesystem::path m_path;
    std::filesystem::file_time_type m_lastWriteTime;
};

}  // namespace Toki
