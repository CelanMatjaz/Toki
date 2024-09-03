if os.host() == "linux" then
    LINUX_DISPLAY_SERVER = os.getenv("XDG_SESSION_TYPE")
end

VULKAN_SDK = os.getenv("VULKAN_SDK")

outputdir = "%{cfg.system}-%{cfg.architecture}"

function set_target_and_object_dirs()
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{cfg.buildcfg}")
    objdir("%{wks.location}/obj/" .. outputdir)
end

function build_options()
    filter "action:vs*"
        cppdialect "C++latest"
        buildoptions "/Zc:preprocessor /std:c++latest"

    filter "toolset:clang or toolset:gcc"
        buildoptions "-std=c++23"

    filter { "system:windows", "toolset:gcc" }
        links { "stdc++exp" }
end

function configuration_configs()
    filter "Debug"
        defines { "TK_DEBUG" }
        symbols "On"
        runtime "Debug"
        optimize "Debug"

    filter {"Debug", "toolset:clang or toolset:gcc" }
        linkoptions "-g"

    filter "Release"
        defines { "TK_NDEBUG", "TK_RELEASE" }
        symbols "Off"
        runtime "Release"
        optimize "On"

    filter "Dist"
        defines { "TK_NDEBUG", "TK_DIST" }
        symbols "Off"
        runtime "Release"
        optimize "Speed"

    filter "system:windows"
        defines { "TK_PLATFORM_WINDOWS" }
        links { "user32", "gdi32", "shell32" }

    filter "system:linux"
        defines { "TK_PLATFORM_LINUX" }
end

function link_vulkan()
    filter "system:windows"
        includedirs { path.join(VULKAN_SDK, "Include") }
        libdirs { path.join(VULKAN_SDK, "Lib") }
        links { "vulkan-1" }

    filter "system:linux"
        includedirs { path.join(VULKAN_SDK, "include") }
        libdirs { path.join(VULKAN_SDK, "lib") }
        links { "vulkan" }
end
