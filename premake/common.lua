require "options"

function CommonOptions(proj)
    filter {}

    language "C++"
    cppdialect "C++23"

    filter { "toolset:msc or action:vs*" }
    buildoptions { "/Zc:preprocessor", "/std:c++latest" }
    flags { "MultiProcessorCompile" }
    defines {
        "_CRT_SECURE_NO_WARNINGS",
        "_CRT_NONSTDC_NO_WARNINGS"
    }

    filter { "toolset:clang or gcc" }
    buildoptions "--std=c++23"

    filter { "toolset:clang or gcc" }
    buildoptions "-ftrivial-auto-var-init=pattern"

    filter { "platforms:Windows" }
    defines {
        "TK_PLATFORM_WINDOWS",
        "_WIN32",
        "NOMINMAX",
        "WIN32_LEAN_AND_MEAN",
    }
    links {
        "user32",
        "gdi32",
        "shell32",
        "kernel32",
        "pathcch",
        "Shlwapi",
    }

    -- Conditionally link libs, primarily from VulkanSDK
    if proj.extra_links ~= nil and type(proj.extra_links) == "table" then
        links(proj.extra_links)
    end

    filter { "platforms:Linux" }
    defines { "TK_PLATFORM_LINUX" }

    filter { "configurations:Debug" }
    defines { "TK_DEBUG" }
    symbols "On"
    runtime "Debug"
    optimize "Debug"

    filter { "configurations:Release or Dist" }
    defines { "TK_NDEBUG" }
    symbols "Off"

    filter { "configurations:Release" }
    defines { "TK_RELEASE" }
    runtime "Release"
    optimize "On"

    filter { "configurations:Dist" }
    defines { "TK_DIST" }
    runtime "Release"
    optimize "Speed"

    filter {}

    includedirs { path.join(proj.dir, "src") }
    files { path.join(proj.dir, "src/**.h") }

    -- Use proj.sources if defined, else add all .cpp files
    if proj.sources ~= nil then
        for _, source in ipairs(proj.sources) do
            files { path.join(proj.dir, source) }
        end
    else
        files { path.join(proj.dir, "src/**.cpp") }
    end

    local outputdir = "%{cfg.platform}-%{cfg.architecture}/%{cfg.buildcfg}"

    targetdir(path.join("%{wks.location}/build/bin", outputdir))
    objdir(path.join("%{wks.location}/build/obj", outputdir, "%{prj.name}"))

    enablewarnings { "all" }

    defines { ("TK_MAX_WINDOW_COUNT=" .. _OPTIONS["maxwindows"]) }

    if proj.extra_defines ~= nil and type(proj.extra_defines) == "table" then
        defines(proj.extra_defines)
    end

    filter {}
end

function PrecompiledHeader(path_without_extension, dir)
    filter {}
    print(path.getabsolute(dir))
    print(path_without_extension)

    pchheader(path.join("src", (path_without_extension .. ".h")))

    filter { "toolset:msc or action:vs*" }
    pchsource(path.join("src", (path_without_extension .. ".cpp")))
    -- files(path_without_extension .. ".cpp")
    -- MSVC hack to include pch in every file automatically
    buildoptions("/FI" .. path.join("src", (path_without_extension .. ".h")))

    filter {}
end

function LinkVulkan(options)
    local VULKAN_SDK = os.getenv("VULKAN_SDK")

    if (VULKAN_SDK == "") then
        error("VULKAN_SDK env variable not found")
        os.exit(-1)
    end

    libdirs { path.join(VULKAN_SDK, "lib") }

    if options.includes == true then
        includedirs { path.join(VULKAN_SDK, "include") }
    end

    if options.link == true then
        filter { "platforms:Windows" }
        links { "vulkan-1" }

        filter { "platforms:Linux" }
        links { "vulkan" }
    end

    filter {}
end

function ProjectLibrary(proj)
    filter {}

    project(proj.name)
    kind(proj.kind)

    basedir(path.getabsolute(proj.dir))

    if proj.vulkan_sdk_options ~= nil then
        LinkVulkan(proj.vulkan_sdk_options)
    end

    CommonOptions(proj)

    filter {}
end

function ProjectExecutable(proj)
    filter {}

    project(proj.name)
    kind "ConsoleApp"

    basedir(path.getabsolute(proj.dir))

    filter { "configurations:Dist" }
    kind "WindowedApp"

    filter { "action:vs*" }
    debugargs "."

    filter {}

    CommonOptions(proj)
    LinkVulkan(proj.vulkan_sdk_options)

    filter {}
end
