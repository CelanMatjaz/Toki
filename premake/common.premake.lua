VULKAN_SDK = os.getenv("VULKAN_SDK")

outputdir = "%{cfg.platform}-%{cfg.architecture}/%{cfg.buildcfg}"

function set_target_and_object_dirs()
    targetdir("%{wks.location}/bin/" .. outputdir)
    objdir("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")
end

function build_options()
    language "C++"
    architecture "x64"

    filter "action:vs*"
        cppdialect "C++latest"
        buildoptions { "/Zc:preprocessor", "/std:c++latest" }
        flags { "MultiProcessorCompile" }

    filter "toolset:clang or toolset:gcc"
        buildoptions "-std=c++23"

    filter {}
end

function configuration_configs()
    defines { "TK_WINDOW_SYSTEM_GLFW" }

    filter "configurations:debug"
        defines { "TK_DEBUG" }
        symbols "On"
        runtime "Debug"
        optimize "Debug"

    filter { "configurations:debug", "toolset:clang or toolset:gcc" }
        linkoptions "-g"

    filter "configurations:release"
        defines { "TK_NDEBUG", "TK_RELEASE" }
        symbols "Off"
        runtime "Release"
        optimize "On"

    filter "configurations:dist"
        defines { "TK_NDEBUG", "TK_DIST" }
        symbols "Off"
        runtime "Release"
        optimize "Speed"

    filter "platforms:windows"
        system "windows"
        defines { "TK_PLATFORM_WINDOWS" }
        links { "user32", "gdi32", "shell32" }

    filter "platforms:linux_wayland or platforms:linux_x11"
        system "linux"
        defines { "TK_PLATFORM_LINUX" }

    filter "platforms:linux_wayland"
        defines { "TK_WAYLAND" }

    filter "platforms:linux_x11"
        defines { "TK_X11" }

    filter {}
end

function link_vulkan()
    filter "platforms:windows"
        includedirs { path.join(VULKAN_SDK, "Include") }
        libdirs { path.join(VULKAN_SDK, "Lib") }
        links { "vulkan-1", "spirv-cross-core", "shaderc_shared", "spirv-cross-glsl" }

    filter "platforms:linux_wayland or platforms:linux_x11"
        includedirs { path.join(VULKAN_SDK, "include") }
        libdirs { path.join(VULKAN_SDK, "lib") }
        links { "vulkan", "spirv-cross-core", "shaderc_shared", "spirv-cross-glsl" }

    filter "platforms:linux_wayland"
        defines { "VK_USE_PLATFORM_WAYLAND_KHR" }

    filter "platforms:linux_x11"
        defines { "VK_USE_PLATFORM_X11_KHR" }

    filter {}
end

function add_files()
    files { "src/**.h", "src/**.cpp" }
end
