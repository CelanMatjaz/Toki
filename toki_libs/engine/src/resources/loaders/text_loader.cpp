#include "text_loader.h"

#include <fstream>

#include "core/assert.h"

namespace toki {

namespace loaders {

std::string read_text_file(std::filesystem::path path) {
    std::ifstream file(path, std::ios::in | std::ios::ate);
    TK_ASSERT(file.is_open(), "File {} was not opened", path);

    std::streampos length = file.tellg();
    file.seekg(0);
    std::string buf(length, 0);
    file.read(buf.data(), length);

    return buf;
}

}  // namespace loaders

}  // namespace toki
