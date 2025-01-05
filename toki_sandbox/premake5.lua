project "Sandbox"
    kind "ConsoleApp"

    links { "Engine", "GLFW" }
    includedirs {
        "%{wks.location}/toki_engine/src/toki",
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
