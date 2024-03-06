#include "text_loader.h"

#include <fstream>

#include "toki/resources/resource_utils.h"

namespace Toki {

std::expected<std::string, Error> TextLoader::readTextFile(std::filesystem::path filePath) {
    if (!ResourceUtils::fileExists(filePath)) {
        return std::unexpected<Error>{ Error::FileNotFoundError };
    }

    std::ifstream shaderFile(filePath, std::ios::ate);

    if (!shaderFile.is_open() || !shaderFile.good()) {
        return std::unexpected<Error>{ Error::FileReadError };
    }

    uint32_t fileSize = shaderFile.tellg();
    shaderFile.seekg(0);

    std::string buf(fileSize, 0);
    shaderFile.read(buf.data(), fileSize);

    return buf;
}

}  // namespace Toki
