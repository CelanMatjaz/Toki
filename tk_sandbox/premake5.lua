project "TK_Sandbox"
    kind "ConsoleApp"
    handleCppDialect()
    targetAndObjectDirs()

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
        symbols "Off"
        defines { "TK_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "TK_NDEBUG", "TK_RELEASE" }
