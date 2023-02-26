VULKAN_SDK = os.getenv("VULKAN_SDK")

workspace "Toki"
    architecture "x86_64"
    language "C++"
    cppdialect "C++20"
    configurations { "Debug", "Release" }
    platforms { "Win64" }

project "Toki"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir("bin/%{cfg.buildcfg}/%{prj.name}")

    -- prebuildcommands { "make -f ./shaders.make" } TODO: move this

    files {
        "src/**.cpp",
    }

    includedirs {
        "./src",
        VULKAN_SDK .. "/Include",
        "./vendor/glfw/include",
        "./vendor/glfw/include",
        "./vendor/glm/",
        "./vendor/imgui",
        "./vendor/imgui/backends",
    }

    libdirs { 
        VULKAN_SDK .. "/Lib",
        "./vendor/glfw/bin/Release",
        "./vendor/imgui/bin/Release",
    }

    links {
        "imgui",
        "vulkan-1",
        "glfw",
    }

    filter "system:windows" 
        links {
            "gdi32",
        }

    filter "configurations:Debug"
        optimize "Debug"
        symbols "On"
        defines { "DEBUG", "UNICODE" }

    filter "configurations:Release"
        defines { "NDEBUG", "UNICODE" }
        optimize "On"        

    filter "action:vs*"  -- for Visual Studio actions
        pchheader "./src/tkpch.h"
        pchsource "./src/tkpch.cpp"

    filter "action:not vs*"  -- for everything else
        pchheader "./src/tkpch.h"