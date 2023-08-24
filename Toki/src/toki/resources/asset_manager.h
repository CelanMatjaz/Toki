// #pragma once

// #include "core/core.h"
// #include "filesystem"

// namespace Toki {

//     enum class AssetType {
//         None,
//         Shader,
//         Texture,
//     };

//     class AssetManager {
//     private:
//         AssetManager();

//     public:
//         const AssetManager* get();

//         void loadAsset(AssetType assetType, std::filesystem::path assetPath);

//         // Asets paths
//         const std::filesystem::path getAssetsPath() const { return assetsPath; }
//         const std::filesystem::path getShadersPath() const { return shadersPath; }
//         const std::filesystem::path getTexturesPath() const { return texturesPath; }

//         // Cache paths
//         const std::filesystem::path getCachePath() const { return cachePath; }
//         const std::filesystem::path getShadersSCachePath() const { return shadersCachePath; }

//     private:
//         std::filesystem::path assetsPath;
//         std::filesystem::path shadersPath;
//         std::filesystem::path texturesPath;

//         std::filesystem::path cachePath;
//         std::filesystem::path shadersCachePath;

//         Scope<AssetManager> assetManager;
//     };

// }