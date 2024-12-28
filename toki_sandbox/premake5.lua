project "Sandbox"
    kind "ConsoleApp"

    links { "Engine", "GLFW" }
    includedirs {
        "%{wks.location}/toki_engine/src",
        "%{wks.location}/toki_engine/include",
    }

    add_files()

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
    link_vulkan()

    filter "system:windows"
        links { "gdi32", "user32", "shell32" }
