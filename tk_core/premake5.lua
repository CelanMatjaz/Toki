project "TK_Core"
    buildoptions "-std=c++23"
    language "C++"
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

    files { "src/**.h", "src/**.cpp" }
    links { "TK_Renderer" }
    includedirs { "%{wks.location}/tk_renderer/src" }

    filter "configurations:Debug"
        kind "SharedLib"
        defines { "TK_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        kind "StaticLib"
        defines { "TK_NDEBUG", "TK_RELEASE" }
        symbols "Off"
        optimize "On"
