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

    pchheader "src/tkpch-renderer.h"

    filter { "action:vs*" }
        pchsource "src/tkpch_renderer.cpp"
        buildoptions "/FIsrc/tkpch_renderer.h"
        includedirs { "." }

    filter {}
