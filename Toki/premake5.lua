project "Toki"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
	staticruntime "off"
    

    targetdir(projectOutputDir)
    objdir(projectOutputDirObj)

    files {
        "src/**.cpp",
        "src/**.h",
        "vendor/stb/std_image.h",
        "vendor/glm/glm/**.hpp",
        "vendor/glm/glm/**.inl"
    }

    includedirs {
        "src",
        includes["vulkan"],
        includes["glfw"],
        includes["glm"],
        includes["imgui"],
        includes["imgui-backends"],
        includes["stb"]
    }

    libdirs { 
        linkDirs["vulkan"],
        linkDirs["glfw"],
        linkDirs["imgui"],
    }

    links {
        linkLibs["vulkan"],
        linkLibs["glfw"],
        linkLibs["imgui"]
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
