local VULKAN_SDK = os.getenv("VULKAN_SDK")

project "TK_Renderer"
    buildoptions "-std=c++23"
    language "C++"
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

    files { "src/**.h", "src/**.cpp" }
    includedirs { 
        VULKAN_SDK .. "/include"
    }
    libdirs {
        VULKAN_SDK .. "/lib"
    }
    links {
        "vulkan"
    }

    filter "configurations:Debug"
        kind "SharedLib"
        defines { "TK_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        kind "StaticLib"
        defines { "TK_NDEBUG", "TK_RELEASE" }
        symbols "Off"
        optimize "On"
