project "Sandbox"
    kind "ConsoleApp"

    links { "Engine", "glfw", "yaml-cpp", "freetype" }
    includedirs {
        "%{wks.location}/toki_engine/src/toki",
        "%{wks.location}/vendor/glm",
    }

    add_files()

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
    link_vulkan()

    filter "system:windows"
        links { "gdi32", "user32", "shell32" }

    filter "action:vs*"
        debugargs "."
