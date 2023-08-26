#include "tkpch.h"
#include "core/assert.h"
#include "shader_loader.h"
#include "fstream"
#include "shaderc/shaderc.hpp"
#include "renderer/shader.h"
#include "functional"
#include "resources/asset_manager.h"
#include "shaderc/shaderc.hpp"

namespace Toki {

    static auto getSubstringAfterNewline = [](const std::string& str) {
        return str.substr(str.find_first_of('\n'));
    };

    static shaderc_shader_kind getShadercShaderKind(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex: return shaderc_shader_kind::shaderc_glsl_vertex_shader;
            case ShaderStage::Fragment: return shaderc_shader_kind::shaderc_glsl_fragment_shader;
        }

        return shaderc_shader_kind::shaderc_glsl_infer_from_source;
    }

    std::vector<std::string> ShaderLoader::load(std::filesystem::path path) {
        std::vector<std::string> code(std::to_underlying(ShaderStage::SHADER_STAGE_MAX_ENUM));
        std::string buffer = loadShaderSourceFile(path);

        std::vector<uint32_t> typePositions;

        for (uint32_t i = 0; i < buffer.size(); ++i) {
            if (buffer[i] != '#') continue;
            const std::string buf(&buffer[i]);
            if (buf.starts_with("#type")) {
                typePositions.push_back(i);
            }
        }

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

    std::unordered_map<ShaderStage, std::vector<uint32_t>> ShaderLoader::loadCompiledBinaries(std::filesystem::path path) {
        auto filename = path.filename();
        auto shaderCachePath = AssetManager::get()->getShadersCachePath();

        std::string shaderBasename = path.stem().string();

#ifndef NDEBUG
        shaderBasename += "-debug";
#endif

        std::string shaderSource = loadShaderSourceFile(path);
        auto shaderSourceHash = std::hash<std::string>{}(shaderSource);

        std::unordered_map<ShaderStage, std::vector<uint32_t>> compiledBinaries;

        // Create cache dir if it doesn't exist
        if (!std::filesystem::exists(shaderCachePath))
            std::filesystem::create_directories(shaderCachePath);

        // Iterate through files in cache/shaders to find if compiled shader spirv is saved
        for (const auto& f : std::filesystem::directory_iterator(shaderCachePath)) {
            const auto file = std::filesystem::path(f);

            if (file.filename().string().starts_with(shaderBasename)) {
                std::ifstream shaderBin(file, std::ios::binary);
                uint64_t shaderHash;
                shaderBin.read((char*) &shaderHash, sizeof(shaderHash));

                if (shaderHash == shaderSourceHash) {
                    uint8_t stageCount;
                    shaderBin.read((char*) &stageCount, sizeof(stageCount));

                    uint8_t stageIndex;
                    uint32_t spirvSize;

                    for (uint32_t i = 0; i < stageCount; ++i) {
                        shaderBin.read((char*) &stageIndex, sizeof(stageIndex));
                        shaderBin.read((char*) &spirvSize, sizeof(spirvSize));
                        compiledBinaries[(ShaderStage) stageIndex] = { };
                        compiledBinaries[(ShaderStage) stageIndex].resize(spirvSize / 4);
                        shaderBin.read((char*) compiledBinaries[(ShaderStage) stageIndex].data(), spirvSize);
                    }

                    return compiledBinaries;
                }

                break;
            }
        }

        auto shaderSources = parseShaderSource(shaderSource);

        shaderc::Compiler spirvCompiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);

#ifdef NDEBUG
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
        options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

        for (const auto& [stage, sourceString] : shaderSources) {
            shaderc::SpvCompilationResult spirvModule = spirvCompiler.CompileGlslToSpv(sourceString, getShadercShaderKind(stage), (const char*) path.c_str(), options);
            TK_ASSERT(spirvModule.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success, std::format("Error compiling shader code from file {}\n\t{}", std::filesystem::absolute(path).string(), spirvModule.GetErrorMessage()));
            compiledBinaries[stage] = { spirvModule.begin(), spirvModule.end() };
        }

        std::string outputBinFile = shaderBasename + ".bin";

        std::ofstream output(shaderCachePath / outputBinFile, std::ios::binary);

        // Write hash
        output.write((char*) &shaderSourceHash, sizeof(shaderSourceHash));

        // Write number of spirv stages
        uint8_t stageCount = compiledBinaries.size();
        output.write((char*) &stageCount, sizeof(uint8_t));

        // Write spirv
        for (const auto& [stage, spirv] : compiledBinaries) {
            uint8_t stageIndex = std::to_underlying(stage);
            uint32_t spirvSize = spirv.size() * 4;
            output.write((char*) &stageIndex, sizeof(stageIndex));
            output.write((char*) &spirvSize, sizeof(spirvSize));
            output.write((char*) spirv.data(), spirvSize);
        }

        return compiledBinaries;
    }


    std::unordered_map<ShaderStage, std::string> ShaderLoader::parseShaderSource(const std::string& source) {
        std::unordered_map<ShaderStage, std::string> sourceStrings;

        std::vector<uint32_t> typePositions;

        for (uint32_t i = 0; i < source.size(); ++i) {
            if (source[i] != '#') continue;
            const std::string buf(&source[i]);
            if (buf.starts_with("#type")) {
                typePositions.push_back(i);
            }
        }

        typePositions.push_back(source.size());

        for (uint32_t i = 0; i < typePositions.size() - 1; ++i) {
            std::string block(&source[typePositions[i]]);
            block = block.substr(0, typePositions[i + 1] - typePositions[i]);

            if (block.starts_with("#type vert") || block.starts_with("#type vertex")) {
                sourceStrings[ShaderStage::Vertex] = getSubstringAfterNewline(block);
            }
            else if (block.starts_with("#type frag") || block.starts_with("#type fragment")) {
                sourceStrings[ShaderStage::Fragment] = getSubstringAfterNewline(block);
            }
        }

        return sourceStrings;
    }

    std::string ShaderLoader::loadShaderSourceFile(std::filesystem::path path) {
        std::string buffer;

        std::ifstream shaderFile(path, std::ios::in | std::ios::ate);
        uint32_t filesize = shaderFile.tellg();
        shaderFile.seekg(0);

        TK_ASSERT(shaderFile.good(), std::format("Error reading file - {}", path.string()));

        std::vector<std::string> code(std::to_underlying(ShaderStage::SHADER_STAGE_MAX_ENUM));
        buffer.resize(filesize);
        shaderFile.read(buffer.data(), filesize);
        buffer = buffer.substr(0, filesize);

        return std::move(buffer);
    }
}


/*

        shaderc::Compiler spirvCompiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

    #ifdef NDEBUG
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
    #else
        options.SetOptimizationLevel(shaderc_optimization_level_zero);
    #endif

        options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);

        shaderc::SpvCompilationResult spirvModule = spirvCompiler.CompileGlslToSpv(shaderCode.data(), getShadercShaderKind(stage), "Shader", options);
        TK_ASSERT(spirvModule.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success, std::format("Error compiling shader code from file {}\n\t{}", std::filesystem::absolute(config.path).string(), spirvModule.GetErrorMessage()));

*/