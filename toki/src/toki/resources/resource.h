#pragma once

#include <expected>
#include <filesystem>

#include "toki/core/errors.h"
#include "toki/core/id.h"

namespace Toki {

#define CREATE_RESOURCE_TYPES(...)       \
    enum class ResourceType : uint32_t { \
        __VA_ARGS__                      \
    };

#define XMACRO(X) X(Unknown) X(ShaderSource) X(ShaderBinary) X(Texture) X(Font) X(AttachmentsConfig) X(ShaderConfig)

#define TO_ENUM(NAME) NAME,
#define TO_STRING(NAME) #NAME,

enum class ResourceType {
    XMACRO(TO_ENUM)
};

static const char* RESOURCE_TYPE_STRINGS[] = { XMACRO(TO_STRING) };

#undef XMACRO
#undef TO_ENUM
#undef TO_STRING

[[nodiscard]] ResourceType getResourceTypeFromPath(const std::filesystem::path& path);

using ResourceHandle = Id;

struct ResourceData {
    ResourceType type;
    std::filesystem::path path;
};

class ResourceSystem;

class Resource {
    friend ResourceSystem;

public:
    Resource() = default;
    Resource(ResourceHandle handle, const ResourceData& data);
    ~Resource() = default;

    [[nodiscard]] const ResourceHandle& getHandle() const;
    [[nodiscard]] ResourceType getType() const;
    [[nodiscard]] std::filesystem::path getPath() const;
    [[nodiscard]] std::filesystem::file_time_type getLastWriteTime() const;
    [[nodiscard]] std::expected<bool, Error> checkForNewWrite() const;

    operator bool() const;

    template <typename T>
        requires std::is_base_of_v<Resource, T>
    const T* as() const {
        return (T*) this;
    }

protected:
    ResourceHandle m_handle;
    ResourceType m_type;
    std::filesystem::path m_path;
    std::filesystem::file_time_type m_lastWriteTime;
};

}  // namespace Toki
