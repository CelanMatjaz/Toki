project "Engine"
    kind "StaticLib"

    includedirs {
        "src",
        "../core/include",
        "../renderer/include"
    }

    files {
        "src/engine.cpp",
        "src/window.cpp",
        "src/toki_entry.cpp",
    }

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
    link_vulkan()

    pch("src/tkpch_engine")

    filter {}
