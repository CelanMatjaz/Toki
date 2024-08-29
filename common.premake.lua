if os.host() == "linux" then
    LINUX_DISPLAY_SERVER = os.getenv("XDG_SESSION_TYPE")
end

VULKAN_SDK = os.getenv("VULKAN_SDK")

function targetAndObjectDirs()
	targetdir ("%{wks.location}/bin/" .. outputdir)
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")
end

function handleCppDialect()
    language "C++"
    if string.startswith(_ACTION, "vs") then
        cppdialect "C++latest"
        buildoptions "/Zc:preprocessor"
    else
        buildoptions "-std=c++23"
    end
end

function handleDefaultLibConfiguration()
    filter "configurations:Debug"
        kind "StaticLib"
        defines { "TK_DEBUG" }
        symbols "On"

        runIfOS("windows", function ()
            targetextension ".lib"
        end)

    filter "configurations:Release"
        kind "StaticLib"
        defines { "TK_NDEBUG", "TK_RELEASE" }
        symbols "Off"
        optimize "On"

        runIfOS("windows", function ()
            targetextension  ".lib"
        end)
end

function runIfOS(requiredOS, fn)
    filter { "system:" .. requiredOS }
    fn()
end

function runIfDisplayServerLinux(displayServer, fn)
    filter "system:linux"
    if LINUX_DISPLAY_SERVER == displayServer then
        fn()
    end
end
