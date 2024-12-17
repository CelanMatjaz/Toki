VULKAN_SDK = os.getenv("VULKAN_SDK")

outputdir = "%{cfg.system}-%{cfg.architecture}"

function set_target_and_object_dirs()
    targetdir("%{wks.location}/bin/" .. outputdir)
    objdir("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")
end

function build_options()
    language "C++"
    architecture "x64"

    filter "action:vs*"
        cppdialect "C++latest"
        buildoptions "/Zc:preprocessor /std:c++latest"

    filter "toolset:clang or toolset:gcc"
        buildoptions "-std=c++23"

    filter {}
end

function configuration_configs()
    filter "configurations:debug"
        defines { "TK_DEBUG" }
        symbols "On"
        runtime "Debug"
        optimize "Debug"

    filter {"configurations:debug", "toolset:clang or toolset:gcc" }
        linkoptions "-g"

    filter "configurations:release"
        defines { "TK_NDEBUG", "TK_RELEASE" }
        symbols "Off"
        runtime "Release"
        optimize "On"

    filter "configurations:dist"
        defines { "TK_NDEBUG", "TK_DIST" }
        symbols "Off"
        runtime "Release"
        optimize "Speed"

    filter "platforms:windows"
        system "windows"
        defines { "TK_PLATFORM_WINDOWS" }
        links { "user32", "gdi32", "shell32" }

    filter "platforms:linux"
        system "linux"
        defines { "TK_PLATFORM_LINUX" }

    filter {}
end

function link_vulkan()
    filter "system:windows"
        includedirs { path.join(VULKAN_SDK, "Include") }
        libdirs { path.join(VULKAN_SDK, "Lib") }
        links { "vulkan-1" }

    filter "system:linux"
        includedirs { path.join(VULKAN_SDK, "include") }
        libdirs { path.join(VULKAN_SDK, "lib") }
        links { "vulkan" }

    filter {}
end

function add_files()
    files { "src/**.h", "src/**.cpp" }
    removefiles { "src/platform/**.h", "src/platform/**.cpp" }
    includedirs { "src" }
end

function add_platform_files()
    filter "system:windows"
        files { "src/platform/windows/**.h", "src/platform/windows/**.cpp" }

    filter "system:linux"
        files { "src/platform/linux/**.h", "src/platform/linux/**.cpp" }

    filter {}
end

