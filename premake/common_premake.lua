VULKAN_SDK = os.getenv("VULKAN_SDK")

outputdir = "%{cfg.platform}-%{cfg.architecture}/%{cfg.buildcfg}"

function set_target_and_object_dirs()
    targetdir("%{wks.location}/.bin/" .. outputdir)
    objdir("%{wks.location}/.obj/" .. outputdir .. "/%{prj.name}")
end

function build_options()
    language "C++"
    architecture "x64"

    filter "action:vs*"
    cppdialect "C++latest"
    buildoptions { "/Zc:preprocessor", "/std:c++latest" }
    flags { "MultiProcessorCompile" }
    defines { "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_WARNINGS" }

    filter "toolset:clang or toolset:gcc"
    buildoptions "--std=c++23 -ftrivial-auto-var-init=pattern -Wall -Werror -Wextra -Wpedantic -Wunused"

    filter {}
end

function configuration_configs_libs()
    filter "configurations:Debug"
    defines { "TK_DEBUG" }
    symbols "On"
    runtime "Debug"
    optimize "Debug"

    filter "configurations:Release"
    defines { "TK_NDEBUG", "TK_RELEASE" }
    symbols "Off"
    runtime "Release"
    optimize "On"

    filter "configurations:Dist"
    defines { "TK_NDEBUG", "TK_DIST" }
    symbols "Off"
    runtime "Release"
    optimize "Speed"

    filter {}
end

function configuration_configs()
    defines { "TK_WINDOW_SYSTEM_GLFW" }

    filter "configurations:Debug"
    defines { "TK_DEBUG" }
    symbols "On"
    runtime "Debug"
    optimize "Debug"
    staticruntime "Off"

    filter { "configurations:Debug", "toolset:clang or toolset:gcc" }
    linkoptions "-g"

    filter "configurations:Release"
    defines { "TK_NDEBUG", "TK_RELEASE" }
    symbols "Off"
    runtime "Release"
    optimize "On"
    staticruntime "Off"

    filter "configurations:Dist"
    defines { "TK_NDEBUG", "TK_DIST" }
    symbols "Off"
    runtime "Release"
    optimize "Speed"
    staticruntime "Off"

    filter "platforms:Windows"
    system "windows"
    defines { "TK_PLATFORM_WINDOWS", "NOMINMAX" }
    links { "user32", "gdi32", "shell32" }

    filter "platforms:Linux"
    system "linux"
    defines { "TK_PLATFORM_LINUX" }

    enablewarnings { "all" }
    fatalwarnings { "all" }

    filter {}
end

function link_vulkan()
    filter { "platforms:Windows", "configurations:Debug" }
    links {
        "spirv-cross-cored",
        "spirv-cross-cppd",
        "spirv-cross-glsld",
        "spirv-cross-reflectd",
        "shadercd",
        "shaderc_combinedd"
    }

    filter { "platforms:Windows", "configurations:not Debug" }
    links {
        "spirv-cross-core",
        "spirv-cross-cpp",
        "spirv-cross-glsl",
        "spirv-cross-reflect",
        "shaderc",
        "shaderc_combined"
    }

    filter { "platforms:Linux" }
    links {
        "spirv-cross-core",
        "spirv-cross-cpp",
        "spirv-cross-glsl",
        "spirv-cross-reflect",
        "shaderc",
        "shaderc_combined"
    }

    filter "platforms:Windows"
    includedirs { path.join(VULKAN_SDK, "Include") }
    libdirs { path.join(VULKAN_SDK, "Lib") }
    links { "vulkan-1" }

    filter "platforms:Linux"
    includedirs { path.join(VULKAN_SDK, "include") }
    libdirs { path.join(VULKAN_SDK, "lib") }
    links { "vulkan" }
    defines { "TK_X11" }

    filter {}
end

function add_files()
    files { "src/**.h", "src/**.cpp" }
end
