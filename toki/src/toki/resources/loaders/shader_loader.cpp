#include "shader_loader.h"

#include "filesystem"
#include "format"
#include "fstream"
#include "functional"
#include "renderer/renderer_types.h"
#include "shaderc/shaderc.hpp"
#include "toki/core/assert.h"

namespace Toki {

std::unordered_map<ShaderStage, std::vector<uint32_t>> ShaderLoader::loadShader(std::filesystem::path path) {
    std::string source = readShaderFile(path);

    std::string shaderBasename = path.stem().string();
    std::string fileExtention = path.extension().string();

#ifndef NDEBUG
    shaderBasename += "-debug";
#endif  // !NDEBUG

    std::filesystem::path shaderCachePath = std::filesystem::absolute(std::filesystem::current_path()) / "cache" / "shaders";
    if (!std::filesystem::exists(shaderCachePath)) {
        std::filesystem::create_directories(shaderCachePath);
    }

    uint64_t shaderSourceHash = std::hash<std::string>{}(source);
    std::unordered_map<ShaderStage, std::vector<uint32_t>> compiledBinaries;

    for (const auto& f : std::filesystem::directory_iterator(shaderCachePath)) {
        const auto file = std::filesystem::path(f);

        if (file.filename().string().starts_with(shaderBasename)) {
            std::ifstream shaderBin(file, std::ios::binary);
            uint64_t shaderHash;
            shaderBin.read((char*) &shaderHash, sizeof(shaderHash));

            if (shaderHash != shaderSourceHash) break;

            uint8_t stageCount;
            shaderBin.read((char*) &stageCount, sizeof(stageCount));

            uint8_t stageIndex;
            uint32_t spirvSize;

            for (uint32_t i = 0; i < stageCount; ++i) {
                shaderBin.read((char*) &stageIndex, sizeof(stageIndex));
                shaderBin.read((char*) &spirvSize, sizeof(spirvSize));
                compiledBinaries[(ShaderStage) stageIndex] = {};
                compiledBinaries[(ShaderStage) stageIndex].resize(spirvSize / 4);
                shaderBin.read((char*) compiledBinaries[(ShaderStage) stageIndex].data(), spirvSize);
            }

            return compiledBinaries;
        }

        break;
    }

    shaderc::Compiler spirvCompiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

    if (fileExtention == ".glsl") {
        options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);
    } else
        TK_ASSERT(false, "Only .glsl extension for shader is currently supported!");

#ifdef NDEBUG
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

    auto shaderStageSources = parseShaderSource(source);

    auto getShadercShaderKind = [](const ShaderStage& stage) {
        switch (stage) {
            case ShaderStage::Vertex:
                return shaderc_shader_kind::shaderc_glsl_vertex_shader;
            case ShaderStage::Fragment:
                return shaderc_shader_kind::shaderc_glsl_fragment_shader;
            case ShaderStage::None:
                break;
        }

        return shaderc_shader_kind::shaderc_glsl_infer_from_source;
    };

    for (const auto& [stage, sourceString] : shaderStageSources) {
        shaderc::SpvCompilationResult spirvModule =
            spirvCompiler.CompileGlslToSpv(sourceString, getShadercShaderKind(stage), (const char*) path.c_str(), options);
        TK_ASSERT(
            spirvModule.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success,
            std::format(
                "Error compiling shader code: compilation status: {} for stage {} from file {}\n\t{}",
                (int) spirvModule.GetCompilationStatus(),
                std::to_underlying(stage),
                std::filesystem::absolute(path).string(),
                spirvModule.GetErrorMessage()
            )
        );
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

std::string ShaderLoader::readShaderFile(std::filesystem::path path) {
    std::ifstream shaderFile(path, std::ios::ate);

    if (!shaderFile.is_open() || !shaderFile.good()) {
        std::cout << "Error opening file " << std::filesystem::absolute(path) << '\n';
        return {};
    }

    uint32_t fileSize = shaderFile.tellg();
    shaderFile.seekg(0);

    std::string buf(fileSize, 0);
    shaderFile.read(buf.data(), fileSize);

    return buf;
}

std::unordered_map<ShaderStage, std::string> ShaderLoader::parseShaderSource(std::string shaderSource) {
    constexpr auto getStage = [](const std::string& str) {
        if (str.starts_with("type vert") || str.starts_with("type vertex")) return ShaderStage::Vertex;
        if (str.starts_with("type frag") || str.starts_with("type fragment") || str.starts_with("type pixel")) return ShaderStage::Fragment;
        return ShaderStage::None;
    };

    std::unordered_map<ShaderStage, std::string> stages;

    constexpr int8_t token = '$';

    // TODO: rewrite
    for (uint32_t i = 0; i < shaderSource.size(); ++i) {
        if (shaderSource[i] == token) {
            shaderSource[i] = 0;
            ShaderStage stage = getStage(shaderSource.substr(i + 1));

            int32_t nextTokenIndex = shaderSource.find(token, i + 1);

            std::string stageSource = shaderSource.substr(i + 1, nextTokenIndex - i - 1);
            stageSource = stageSource.substr(stageSource.find('\n'));

            uint32_t j = stageSource.size() - 1;
            for (; j >= 0; --j) {
                if (stageSource[j] != ' ' && stageSource[j] != '\n' && stageSource[j] != 0) break;
            }

            stages[stage] = stageSource.substr(0, j + 1);

            i = nextTokenIndex - 1;
        }
    }

    return stages;
}

}  // namespace Toki
