project "TK_Renderer"
    language "C++"
    kind "StaticLib"

    files { "src/**.h", "src/**.cpp" }
    includedirs { "src" }

    links { "TK_Core", "glfw" }
    includedirs {
        "%{wks.location}/tk_core/include",
        "%{wks.location}/vendor/glfw/include",
    }

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
    link_vulkan()
