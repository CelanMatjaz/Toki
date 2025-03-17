project "Renderer"
    kind "StaticLib"

    includedirs {
        "src",
        "../core/include"
    }

    files {
        "src/**.h",
        "src/**.cpp",
    }

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
    link_vulkan()

    pch("src/tkpch_renderer")

    filter {}
