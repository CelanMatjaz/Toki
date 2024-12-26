project "Core"
    kind "StaticLib"

    includedirs {
        "%{wks.location}/vendor/glfw/include",
    }

    add_files()

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
    link_vulkan()
