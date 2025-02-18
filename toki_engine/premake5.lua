project "Engine"
    kind "StaticLib"

    includedirs {
        "src",
        "src/toki",
        "%{wks.location}/vendor/glfw/include",
        "%{wks.location}/vendor/glm",
        "%{wks.location}/vendor/yaml-cpp/include",
        "%{wks.location}/vendor/stb",
        "%{wks.location}/vendor/free_type/include",
    }

    -- add_files()

    files {
        "src/toki/core/**.h",
        "src/toki/platform/**.cpp",
        "src/platform/**.cpp",
        "src/tkpch.h",
        "src/tkpch.cpp",
    }

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
