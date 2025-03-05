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

    pchheader "src/tkpch_engine.h"

    filter { "action:vs*" }
    files("src/tkpch_engine.cpp")
    pchsource "src/tkpch_engine.cpp"
        buildoptions "/FIsrc/tkpch_engine.h"
        includedirs { "." }

    filter {}
