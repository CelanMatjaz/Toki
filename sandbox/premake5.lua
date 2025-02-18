project "Sandbox1"
    kind "ConsoleApp"

    links { "Engine" }
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
        links { "gdi32", "user32", "shell32", "kernel32", "pathcch" }

    filter "action:vs*"
        debugargs "."
        linkoptions { "/SUBSYSTEM:WINDOWS" }
