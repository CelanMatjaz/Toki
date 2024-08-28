project "TK_Sandbox"
    kind "ConsoleApp"
    buildoptions "-std=c++23"
    language "C++"
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

    files {
        "src/**.h",
        "src/**.cpp",
        "src/**.c"
    }
    links { "TK_Core", "TK_Renderer", "GLFW" }
    includedirs {
        "%{wks.location}/tk_core/src",
        "%{wks.location}/tk_sandbox/src",
        "%{wks.location}/tk_sandbox/src/include",
        "%{wks.location}/vendor/glfw/include"
    }

    filter "configurations:Debug"
        defines { "TK_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "TK_NDEBUG", "TK_RELEASE" }
        symbols "Off"
