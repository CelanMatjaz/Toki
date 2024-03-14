#pragma once

#include <filesystem>

namespace Toki {

enum class ResourceType {
    Font
};

class Resource {
public:
    Resource() = default;
    Resource(ResourceType type, std::filesystem::path path);
    ~Resource() = default;

    bool checkForNewWrite() const;

    ResourceType getType() const;
    std::filesystem::path getPath() const;
    std::filesystem::file_time_type getLastWriteTime() const;

private:
    ResourceType m_type;
    std::filesystem::path m_path;
    std::filesystem::file_time_type m_lastWriteTime;
};

}  // namespace Toki
