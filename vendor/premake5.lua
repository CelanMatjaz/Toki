require "utils"

project "GLFW"
    kind "StaticLib"
    language "C"
    targetAndObjectDirs()
    warnings "Off"
    targetextension ".lib"
    staticruntime "On"

    files {
        "glfw/src/**.h",
        "glfw/src/**.c"
    }
    includedirs { 
        "glfw/include",
        "includes/glfw"
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS"
    }

    runIfOS("windows", function ()
        defines { "_GLFW_WIN32" }
    end)

    runIfOS("linux", function ()
        runIfDisplayServerLinux("wayland", function ()
            defines { "_GLFW_WAYLAND" }
        end)
        runIfDisplayServerLinux("x11", function ()
            defines { "_GLFW_X11" }
        end)
    end)

