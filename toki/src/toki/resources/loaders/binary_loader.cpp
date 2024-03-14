#include "binary_loader.h"

#include <fstream>

#include "toki/resources/resource_utils.h"

namespace Toki {

std::expected<std::vector<uint8_t>, Error> BinaryLoader::readBinaryFile(std::filesystem::path filePath) {
    if (!ResourceUtils::fileExists(filePath)) {
        return std::unexpected<Error>{ Error::FileNotFoundError };
    }

    std::ifstream binaryFile(filePath, std::ios::ate | std::ios::binary);

    if (!binaryFile.is_open() || !binaryFile.good()) {
        return std::unexpected<Error>{ Error::FileReadError };
    }

    uint32_t fileSize = binaryFile.tellg();
    binaryFile.seekg(0);

    std::vector<uint8_t> buf(fileSize, 0);
    binaryFile.read((char*) buf.data(), fileSize);

    return buf;
}

}  // namespace Toki