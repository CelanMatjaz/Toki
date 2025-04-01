local projects = {
    executables = {
        sandbox2 = {
            name = "Sandbox",
            dir = "toki_executables/sandbox2",
            deps = { "engine", "core" },
            vulkan_sdk_options = { link = true },
        },
    },

    libraries = {
        core = {
            name = "Core",
            kind = "StaticLib",
            dir = "toki_libs/core",
            pch = "tkpch_core",
        },

        engine = {
            name = "Engine",
            kind = "StaticLib",
            dir = "toki_libs/engine",
            deps = { "renderer", "core" },
            sources = {
                "src/engine.cpp",
                "src/window.cpp",
                "src/toki_entry.cpp",
            }
        },

        renderer = {
            name = "Renderer",
            kind = "StaticLib",
            dir = "toki_libs/renderer",
            deps = { "shader_compiler", "core" },
            vulkan_sdk_options = { includes = true },
        },

        shader_compiler = {
            name = "ShaderCompiler",
            kind = "SharedLib",
            dir = "toki_libs/shader_compiler",
            -- deps = { "core" },
            extra_lib_includes = { "renderer", "core" },
            vulkan_sdk_options = { includes = true },
            extra_links = {
                "spirv-cross-cored",
                "spirv-cross-cppd",
                "spirv-cross-glsld",
                "spirv-cross-reflectd",
                "shadercd",
                "shaderc_combinedd"
            },
            extra_defines = { "TK_SHARED_LIB_EXPORT" },
        },
    }
}

return projects;
