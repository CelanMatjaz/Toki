project "Core"
    kind "StaticLib"

    includedirs {
        "src",
    }

    files {
        "src/**.h",
        "src/**.cpp",
    }

    set_target_and_object_dirs()
    configuration_configs()
    build_options()

    pch("src/tkpch_core")

    filter {}
