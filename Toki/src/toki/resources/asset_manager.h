#pragma once

#include "core/core.h"
#include "filesystem"

namespace Toki {


    class AssetManager {
    private:
        static inline Scope<AssetManager> manager;

    public:
        static const AssetManager* get();

        AssetManager();
        ~AssetManager() = default;

        const std::filesystem::path getAssetsPath() const { return assetsPath; }
        const std::filesystem::path getShadersPath() const { return shadersPath; }
        const std::filesystem::path getTexturesPath() const { return texturesPath; }

        const std::filesystem::path getCachePath() const { return cachePath; }
        const std::filesystem::path getShadersCachePath() const { return shadersCachePath; }

    private:
        // Asset paths
        std::filesystem::path assetsPath;
        std::filesystem::path shadersPath;
        std::filesystem::path texturesPath;

        // Cache paths
        std::filesystem::path cachePath;
        std::filesystem::path shadersCachePath;
    };

}