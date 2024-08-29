project "TK_Sandbox"
    kind "ConsoleApp"
    handleCppDialect()
    targetAndObjectDirs()

    files {
        "src/**.h",
        "src/**.cpp",
        "src/**.c"
    }
    links { "TK_Core", "TK_Engine", "GLFW" }
    includedirs {
        "%{wks.location}/tk_sandbox/src",
        "%{wks.location}/tk_core/include",
        "%{wks.location}/tk_engine/include",
        "%{wks.location}/vendor/glfw/include"
    }

    runIfOS("windows", function ()
        links { "user32", "gdi32", "shell32" }
    end)

    filter "configurations:Debug"
        defines { "TK_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "TK_NDEBUG", "TK_RELEASE" }
        symbols "Off"
