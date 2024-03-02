VULKAN_SDK = os.getenv("VULKAN_SDK")

VULKAN_INCLUDE = path.join(VULKAN_SDK, "Include")
VULKAN_BIN = path.join(VULKAN_SDK, "Lib")

set_project("Toki")

add_rules("mode.debug", "mode.release")
set_arch("x64")
set_languages("cxx23")
set_exceptions("cxx")
set_toolchains("msvc", {vs = "2022"})

target("engine")
    set_default(true)
    set_kind("binary")
    set_group("engine")
    add_files("engine/src/**.cpp")
    add_includedirs("engine/src", "toki/src/toki", "vendor/glm", "vendor/stb", "vendor/spdlog/include", "vendor/imgui")
    set_rundir("engine")
    add_deps("toki")
    set_group("Engine")
    add_defines("GLM_FORCE_RADIANS", "GLM_FORCE_DEPTH_ZERO_TO_ONE")

    if is_mode("debug") then
        add_defines("DEBUG")
        set_symbols("debug")
        set_optimize("none")
        set_runtimes("MTd")
    elseif is_mode("release") then
        add_defines("RELEASE")
        set_optimize("fastest")
        set_symbols("hidden")
        set_runtimes("MT")
    end 

target("glfw")
    set_kind("static")    
    set_group("Dependencies")
    add_includedirs("vendor/glfw/src")
    add_files("vendor/glfw/src/*.c")
    add_defines("_GLFW_WIN32", "_GLFW_VULKAN_STATIC")
    add_links("gdi32", "user32", "shell32")
    set_prefixname("")
    set_extension(".lib")
    set_group("dependencies")

target("imgui")
    set_kind("static")
    set_group("Dependencies")
    add_files("vendor/imgui/*.cpp")
    add_files("vendor/imgui/backends/imgui_impl_glfw.cpp")
    add_files("vendor/imgui/backends/imgui_impl_vulkan.cpp")
    add_includedirs("vendor/imgui", "vendor/glfw/include", VULKAN_INCLUDE)
    set_prefixname("")
    set_extension(".lib")
    set_group("dependencies")
    
target("toki")
    set_kind("static")
    add_files("toki/src/**.cpp")
    set_group("Toki")
    add_headerfiles("toki/src/**.h")
    add_includedirs(
        "toki/src/toki",
        "toki/src/platform",
        "toki/src/renderer",
        "toki/src",
        VULKAN_INCLUDE,
        "vendor/glfw/include",
        "vendor/imgui",
        "vendor/glm",
        "vendor/stb",
        "vendor/spdlog/include"
    )
    add_deps("glfw", "imgui")
    set_pcxxheader("toki/src/tkpch.h")
    set_pmxxheader("toki/src/itkpch.h")
    add_linkdirs(VULKAN_BIN)
    add_links("vulkan-1")
    set_prefixname("")
    set_extension(".lib")
    add_defines("GLM_FORCE_RADIANS", "GLM_FORCE_DEPTH_ZERO_TO_ONE")

    if is_mode("debug") then
        add_defines("DEBUG")
        set_symbols("debug")
        set_optimize("none")
        set_runtimes("MTd")
        add_links("spirv-cross-cored", "shaderc_sharedd", "spirv-cross-glsld")
    elseif is_mode("release") then
        add_defines("DEBUG")
        set_optimize("fastest")
        set_symbols("hidden")
        set_runtimes("MT")
        add_links("spirv-cross-core", "shaderc_shared", "spirv-cross-glsl")
    end
