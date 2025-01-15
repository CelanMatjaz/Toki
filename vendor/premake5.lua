project "GLFW"
    language "C++"
    kind "StaticLib"
    files { "glfw/src/**.c", "glfw/src/**.h" }
    includedirs { "glfw/src" }

    defines { "_GLFW_VULKAN_STATIC" }

    filter "platforms:windows"
        defines { "_GLFW_WIN32" }
        links { "gdi32", "user32", "shell32" }

    filter "platforms:linux_wayland or platforms:linux_x11"
        defines { "_GLFW_WAYLAND", "_GLFW_X11" }
        includedirs { "includes/glfw" }

    set_target_and_object_dirs()
    configuration_configs_libs()

project "yaml-cpp"
    language "C++"
    kind "StaticLib"
    files { "yaml-cpp/src/**.cpp", "yaml-cpp/src/**.cpp" }
    includedirs { "yaml-cpp/src", "yaml-cpp/include" }

    defines { "YAML_CPP_STATIC_DEFINE", "YAML_CPP_API=" }

    set_target_and_object_dirs()
    configuration_configs_libs()
