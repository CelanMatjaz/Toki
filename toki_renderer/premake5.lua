project "Renderer"
    kind "StaticLib"

    links { "GLFW", "Core" }
    includedirs {
        "%{wks.location}/toki_core/include",
        "%{wks.location}/toki_renderer/include",
        "%{wks.location}/vendor/glfw/include",
    }

    add_files()

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
    link_vulkan()
