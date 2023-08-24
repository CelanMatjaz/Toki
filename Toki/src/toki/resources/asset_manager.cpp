#include "tkpch.h"
#include "asset_manager.h"

namespace Toki {

    AssetManager::AssetManager() {
        assetsPath = std::filesystem::absolute(std::filesystem::current_path()) / "assets";
        shadersPath = assetsPath / "shaders";
        texturesPath = assetsPath / "textures";

        cachePath = std::filesystem::current_path() / "cache";
        shadersCachePath = cachePath / "shaders";
    }

    const AssetManager* AssetManager::get() {
        if (!manager.get()) manager = createScope<AssetManager>();
        return manager.get();
    }

}