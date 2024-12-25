project "GLFW"
    language "C++"
    kind "StaticLib"
    files { "glfw/src/**.c", "glfw/src/**.h" }
    includedirs { "glfw/src" }

    symbols "Off"
    runtime "Release"
    optimize "Speed"

    set_target_and_object_dirs()

    defines { "_GLFW_VULKAN_STATIC" }

    filter "system:windows"
        defines { "_GLFW_WIN32" }
        links { "gdi32", "user32", "shell32" }

    filter "system:linux"
        defines { "_GLFW_WAYLAND", "_GLFW_X11" }
