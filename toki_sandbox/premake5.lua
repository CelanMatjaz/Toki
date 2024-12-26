project "Sandbox"
    kind "ConsoleApp"

    links { "Engine", "Renderer", "GLFW", "Core" }
    includedirs {
        "%{wks.location}/toki_core/include",
        "%{wks.location}/toki_renderer/include",
        "%{wks.location}/toki_engine/include",
    }

    add_files()

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
    link_vulkan()

    filter "system:windows"
        links { "gdi32", "user32", "shell32" }
