#include "resource.h"

namespace Toki {

Resource::Resource(ResourceType type, std::filesystem::path path) :
    m_type(type),
    m_path(path),
    m_lastWriteTime(std::filesystem::last_write_time(path)),
    m_size(std::filesystem::file_size(path)) {}

bool Resource::checkForNewWrite() const {
    return m_lastWriteTime < std::filesystem::last_write_time(m_path);
}

void Resource::update() {
    m_lastWriteTime = std::filesystem::last_write_time(m_path);
    m_size = std::filesystem::file_size(m_path);
}

ResourceType Resource::getType() const {
    return m_type;
}

std::filesystem::path Resource::getPath() const {
    return m_path;
}

std::filesystem::file_time_type Resource::getLastWriteTime() const {
    return m_lastWriteTime;
}

}  // namespace Toki
