VULKAN_SDK = os.getenv("VULKAN_SDK")

workspace "toki"
    architecture "x86_64"
    language "C++"
    cppdialect "C++20"
    configurations { "debug", "release" }
    platforms { "win64" }

group "dependencies"
	include "./vendor/glfw"
	include "./vendor/imgui"
	-- include "./vendor/stb"
	-- include "./vendor/glm"
group ""

group "core"
	include "../toki"
group ""

project "toki"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir("bin/%{cfg.buildcfg}/%{prj.name}")

    prebuildcommands { "make -f ./shaders.make" }
    prelinkcommands {
        "make -C ./vendor/glfw config=release_win64",
        "make -C ./vendor/imgui config=release_win64"
    }

    files {
        "src/**.cpp",
        "src/**.h",
    }

    includedirs {
        "./src",
        VULKAN_SDK .. "/Include",
        "./vendor/glfw/include",
        "./vendor/glm/",
        "./vendor/imgui",
        "./vendor/imgui/backends",
        "./vendor/stb/",
    }

    libdirs { 
        VULKAN_SDK .. "/Lib",
        "./vendor/glfw/bin/release",
        "./vendor/imgui/bin/release",
    }

    links {
        "imgui",
        "glfw",
        "vulkan-1",
    }

    filter "system:windows" 
        links {
            "gdi32",
        }

    filter "configurations:debug"
        optimize "Debug"
        symbols "On"
        defines { "DEBUG", "UNICODE" }

    filter "configurations:release"
        defines { "NDEBUG", "UNICODE" }
        optimize "On"        

    filter "action:vs*"  -- for Visual Studio actions
        pchsource "./src/tkpch.cpp"
        pchheader "tkpch.h"

    filter "action:not vs*"  -- for everything else
        pchheader "./src/tkpch.h"