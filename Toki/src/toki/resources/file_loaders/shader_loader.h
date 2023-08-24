#pragma once

#include "vector"
#include "string"
#include "filesystem"
#include "unordered_map"
#include "renderer/shader.h"

namespace Toki {

    class ShaderLoader {
    public:
        static std::vector<std::string> load(std::filesystem::path path);
        static std::vector<uint32_t> loadCompiled(std::filesystem::path path);

        static std::unordered_map<ShaderStage, std::vector<uint32_t>> loadCompiledBinaries(std::filesystem::path path);
        static std::unordered_map<ShaderStage, std::string> parseShaderSource(const std::string& source);

    private:
        static std::string loadShaderSourceFile(std::filesystem::path path);
    };

}