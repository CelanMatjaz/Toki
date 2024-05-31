#include "resource.h"

#include <unordered_map>

#include "toki/core/assert.h"
#include "toki/core/logging.h"
#include "toki/resources/resource_utils.h"

namespace Toki {

static std::unordered_map<std::filesystem::path, ResourceType> s_resourceExtensionMap = {
    { ".jpg", ResourceType::Texture },             //
    { ".png", ResourceType::Texture },             //
    { ".jpeg", ResourceType::Texture },            //
    { ".glsl", ResourceType::ShaderSource },       //
    { ".shdr", ResourceType::ShaderBinary },       //
    { ".ttf", ResourceType::Font },                //
    { ".acfg", ResourceType::AttachmentsConfig },  //
    { ".scfg", ResourceType::ShaderConfig },       //
};

ResourceType getResourceTypeFromPath(const std::filesystem::path& path) {
    TK_ASSERT(path.has_extension(), "Path {} does not have an extension", path.string());

    if (!path.has_extension()) {
        return ResourceType::Unknown;
    }

    if (auto ext = path.extension(); s_resourceExtensionMap.contains(ext)) {
        return s_resourceExtensionMap[ext];
    }

    return ResourceType::Unknown;
}

Resource::Resource(ResourceHandle handle, const ResourceData& data) :
    m_handle(handle),
    m_type(data.type),
    m_path(data.path),
    m_lastWriteTime(std::filesystem::last_write_time(data.path)) {}

const ResourceHandle& Resource::getHandle() const {
    return m_handle;
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

std::expected<bool, Error> Resource::checkForNewWrite() const {
    if (!ResourceUtils::fileExists(m_path)) {
        return std::unexpected(Error::ResourcePathNotFound);
    }

    return m_lastWriteTime < std::filesystem::last_write_time(m_path);
}

Resource::operator bool() const {
    return m_type != ResourceType::Unknown;
}

}  // namespace Toki
