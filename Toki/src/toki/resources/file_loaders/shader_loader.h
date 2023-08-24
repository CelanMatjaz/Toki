#pragma once

#include "vector"
#include "string"
#include "filesystem"

namespace Toki {

    class ShaderLoader {
    public:
        static std::vector<std::string> loadRaw(std::filesystem::path path);
        static std::vector<uint32_t> loadCompiled(std::filesystem::path path);
    };

}