#include "shader_serializer.h"

#include <toki/resources/loaders/text_loader.h>

#include <fstream>

#include "toki/core/logging.h"
#include "toki/resources/directory.h"
#include "toki/resources/resource_types.h"
#include "toki/resources/resource_utils.h"
#include "toki/resources/serializers/metadata.h"

namespace Toki {

Error ShaderSerializer::serialize(std::filesystem::path fileName, std::vector<uint32_t>& spirv) {
    ResourceUtils::ensureDirectory(Directory::cacheDirectory() / "shaders");

    std::filesystem::path savePath = Directory::cacheDirectory() / "shaders" / fileName;
    savePath.replace_extension("shdr");

    std::ofstream file(savePath, std::ios::binary);

    if (!file.is_open()) {
        LOG_ERROR("File could not be opened for saving");
        return Error::FileOpenError;
    }

    Metadata metadata{};
    metadata.resourceType = RESOURCE_TYPE("SHDR");
    metadata.serializerVersion = RESOURCE_TYPE("0001");
    metadata.timeSaved = time(0);

    uint32_t spirvSize = spirv.size() * sizeof(uint32_t);

    file.write((char*) &metadata, sizeof(Metadata));
    file.write((char*) &spirvSize, sizeof(uint32_t));
    file.write((char*) spirv.data(), spirv.size() * sizeof(uint32_t));

    return Error::NoError;
}

std::expected<std::vector<uint32_t>, Error> ShaderSerializer::deserialize(const std::filesystem::path& path) {
    if (!ResourceUtils::fileExists(path)) {
        return std::unexpected(Error::FileNotFoundError);
    }

    std::ifstream file(path, std::ios::binary);

    Metadata metadata{};
    file.read((char*) &metadata, sizeof(Metadata));

    uint32_t spirvSize;
    file.read((char*) &spirvSize, sizeof(uint32_t));

    std::vector<uint32_t> spirv(spirvSize / sizeof(uint32_t));
    file.read((char*) spirv.data(), spirvSize);

    return spirv;
}

Ref<ShaderSource> ShaderSerializer::loadShaderSource(std::filesystem::path path) {
    auto result = CHECK_ERROR(TextLoader::readTextFile(path));

    if (!result.has_value()) {
        return nullptr;
    }

    return createRef<ShaderSource>(std::move(result.value()));
}

Ref<ShaderBinary> ShaderSerializer::loadShaderBinary(std::filesystem::path path) {
    auto result = CHECK_ERROR(ShaderSerializer::deserialize(path));

    if (!result.has_value()) {
        return nullptr;
    }

    return createRef<ShaderBinary>(std::move(result.value()));
}

}  // namespace Toki
