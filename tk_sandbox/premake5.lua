project "TK_Sandbox"
    language "C++"
    kind "ConsoleApp"
    files { "src/**.h", "src/**.cpp" }
    includedirs { "src" }

    links { "TK_Core", "TK_Engine", "TK_Renderer", "glfw" }
    includedirs {
        "%{wks.location}/tk_core/include",
        "%{wks.location}/tk_engine/include",
        "%{wks.location}/vendor/glfw/include",
    }

    links { "user32", "gdi32", "shell32" }

    set_target_and_object_dirs()
    configuration_configs()
    build_options()
    link_vulkan()
