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

    pchheader "src/tkpch_core.h"

    filter { "action:vs*" }
        pchsource "src/tkpch_core.cpp"
        buildoptions "/FIsrc/tkpch_core.h"
        includedirs { "." }

    filter {}
