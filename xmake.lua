VULKAN_SDK = os.getenv("VULKAN_SDK")

VULKAN_INCLUDE = path.join(VULKAN_SDK, "Include")
VULKAN_BIN = path.join(VULKAN_SDK, "Lib")

set_project("Toki")

add_rules("mode.debug", "mode.release")
set_arch("x64")
set_languages("cxx23")
set_exceptions("cxx")
set_toolchains("msvc", {vs = "2022"})

target("demo")
    set_default(true)
    set_kind("binary")
    set_group("demo")
    add_files("Toki-Renderer/src/**.cpp")
    add_installfiles("Toki-Renderer/assets/**")
    add_headerfiles("Toki-Renderer/src/**.h")
    add_includedirs("Toki-Renderer/src", "Toki/src/toki", "vendor/glm", "vendor/stb", "vendor/spdlog/include")
    set_rundir("Toki-Renderer")
    add_deps("toki")
    set_group("demo")
    add_defines("GLM_FORCE_RADIANS", "GLM_FORCE_DEPTH_ZERO_TO_ONE")

    if is_mode("debug") then
        add_defines("DEBUG")
        set_symbols("debug")
        set_optimize("none")
        set_runtimes("MDd")
    end

    if is_mode("release") then
        add_defines("NDEBUG")
        set_optimize("fastest")
        set_symbols("hidden")
        set_runtimes("MD")
    end

target("glfw")
    set_kind("static")    
    set_group("dependencies")
    add_includedirs("vendor/glfw/src")
    add_files("vendor/glfw/src/*.c")
    add_defines("_GLFW_WIN32", "_GLFW_VULKAN_STATIC")
    add_links("gdi32", "user32", "shell32")
    set_prefixname("")
    set_extension(".lib")
    set_group("dependencies")

target("imgui")
    set_kind("static")
    set_group("dependencies")
    add_files("vendor/imgui/*.cpp")
    add_files("vendor/imgui/backends/imgui_impl_glfw.cpp")
    add_files("vendor/imgui/backends/imgui_impl_vulkan.cpp")
    add_includedirs("vendor/imgui", "vendor/glfw/include", VULKAN_INCLUDE)
    set_prefixname("")
    set_extension(".lib")
    set_group("dependencies")
    
target("toki")
    set_kind("static")
    add_files("Toki/src/**.cpp")
    set_group("engine")
    add_headerfiles("Toki/src/**.h")
    add_includedirs("Toki/src", "Toki/src/toki", "Toki/src/platform", VULKAN_INCLUDE, "vendor/glfw/include", "vendor/imgui", "vendor/glm", "vendor/stb", "vendor/spdlog/include")
    add_deps("glfw", "imgui")
    set_pcxxheader("Toki/src/tkpch.h")
    set_pmxxheader("Toki/src/tkpch.h")
    add_linkdirs(VULKAN_BIN)
    add_links("vulkan-1")
    set_prefixname("")
    set_extension(".lib")
    add_defines("GLM_FORCE_RADIANS", "GLM_FORCE_DEPTH_ZERO_TO_ONE")

    if is_mode("debug") then
        add_defines("DEBUG")
        set_symbols("debug")
        set_optimize("none")
        set_runtimes("MDd")
        add_links("spirv-cross-cored", "shaderc_sharedd", "spirv-cross-glsld")
    end

    if is_mode("release") then
        add_defines("NDEBUG")
        set_optimize("fastest")
        set_symbols("hidden")
        set_runtimes("MD")
        add_links("spirv-cross-core", "shaderc_shared", "spirv-cross-glsl")
    end