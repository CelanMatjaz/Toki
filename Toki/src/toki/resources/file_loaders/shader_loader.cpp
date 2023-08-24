#include "tkpch.h"
#include "core/assert.h"
#include "shader_loader.h"
#include "fstream"
#include "shaderc/shaderc.hpp"
#include "renderer/shader.h"

namespace Toki {

    static auto getSubstringAfterNewline = [](const std::string& str) {
        return str.substr(str.find_first_of('\n'));
    };

    std::vector<std::string> ShaderLoader::loadRaw(std::filesystem::path path) {
        std::ifstream shaderFile(path, std::ios::in | std::ios::ate);
        uint32_t filesize = shaderFile.tellg();
        shaderFile.seekg(0);

        TK_ASSERT(shaderFile.good(), std::format("Error reading file - {}", path.string()));

        std::vector<std::string> code(std::to_underlying(ShaderStage::SHADER_STAGE_MAX_ENUM));
        std::string buffer;
        buffer.resize(filesize);
        shaderFile.read(buffer.data(), filesize);
        buffer = buffer.substr(0, filesize);

        std::vector<uint32_t> typePositions;

        for (uint32_t i = 0; i < filesize; ++i) {
            if (buffer[i] != '#') continue;
            const std::string buf(&buffer[i]);
            if (buf.starts_with("#type")) {
                typePositions.push_back(i);
            }
        }

        typePositions.push_back(filesize);

        for (uint32_t i = 0; i < typePositions.size() - 1; ++i) {
            auto t1 = typePositions[i];
            auto t2 = typePositions[i + 1];

            std::string block(&buffer[typePositions[i]]);
            block = block.substr(0, typePositions[i + 1] - typePositions[i]);

            if (block.starts_with("#type vert") || block.starts_with("#type vertex")) {
                code[std::to_underlying(ShaderStage::Vertex)] = getSubstringAfterNewline(block);
            }
            else if (block.starts_with("#type frag") || block.starts_with("#type fragment")) {
                code[std::to_underlying(ShaderStage::Fragment)] = getSubstringAfterNewline(block);
            }
        }

        return code;
    }

    std::vector<uint32_t> ShaderLoader::loadCompiled(std::filesystem::path path) {
        if (!std::filesystem::exists(path))
            TK_ASSERT(false, std::format("Path to shader does not exist: {}", std::filesystem::absolute(path).string()));

        std::ifstream file(path.string(), std::ios::ate | std::ios::binary);
        uint32_t fileSize = file.tellg();

        std::cout << std::format("{} {} \t - {}\n", fileSize, (fileSize / sizeof(uint32_t)), path.string());

        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
        file.seekg(0);
        file.read((char*) buffer.data(), fileSize);
        file.close();

        return buffer;
    }
}