#include "text.h"

#include <fstream>

#include "core/assert.h"
#include "core/core.h"

namespace toki {

std::string read_text_file(std::filesystem::path path) {
    std::ifstream file(path, std::ios::in | std::ios::ate);
    TK_ASSERT(file.is_open(), "File {} was not opened", path);

    u32 length = file.tellg();
    std::string buf(length, 0);
    file.read(buf.data(), length);

    return buf;
}

}  // namespace toki
