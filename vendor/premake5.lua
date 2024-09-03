require "utils"

project "GLFW"
    language "C"
    kind "StaticLib"
    warnings "Off"
    set_target_and_object_dirs()
    linkoptions "-g"

    files { "glfw/src/**.h", "glfw/src/**.c" }
    includedirs { "glfw/include", "includes/glfw" }

    filter "system:windows"
        defines { "_GLFW_WIN32" }

    filter "system:linux"
        defines { "_GLFW_WAYLAND", "_GLFW_X11" }

    filter "toolset:msc"
        staticruntime "On"

