project "TK_Core"
    language "C++"
    kind "StaticLib"

    files { "src/**.h", "src/**.cpp" }
    includedirs { "src" }

    filter "system:windows"
        links { "" }

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
