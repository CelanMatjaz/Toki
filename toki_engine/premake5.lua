project "Engine"
    kind "StaticLib"

    links { "GLFW" }
    includedirs {
        "src",
        "src/toki",
        "%{wks.location}/vendor/glfw/include",
        "%{wks.location}/vendor/glm",
    }

    add_files()

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
    link_vulkan()

    pchheader "src/tkpch.h"

    filter { "action:vs*" }
        pchsource "src/tkpch.cpp"
        buildoptions "/FIsrc/tkpch.h"
        includedirs { "." }

    filter {}
