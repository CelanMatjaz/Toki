VULKAN_SDK = os.getenv("VULKAN_SDK")

project "Toki"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir(outputdir)

    prebuildcommands { "make -f ./shaders.make" }

    files {
        "src/**.cpp",
        "src/**.h",
    }

    includedirs {
        "src",
        VULKAN_SDK .. "/Include",
        "vendor/glfw/include",
        "vendor/glm/",
        "vendor/imgui",
        "vendor/imgui/backends",
        "vendor/stb/",
    }

    libdirs { 
        VULKAN_SDK .. "/Lib",
        outputdir
    }

    links {
        "vulkan-1",
        "GLFW",
        "ImGui"
    }

    filter "configurations:Debug"
        optimize "Debug"
        symbols "On"
        defines { "DEBUG", "UNICODE" }

    filter "configurations:Release"
        defines { "NDEBUG", "UNICODE" }
        optimize "On"

    filter "action:vs*"
        pchsource "./src/tkpch.cpp"
        pchheader "tkpch.h"

    filter "action:not vs*"
        pchheader "./src/tkpch.h"

    filter "system:windows"
        links {            
            "gdi32"
        }
