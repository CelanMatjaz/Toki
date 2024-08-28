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
    else
        buildoptions "-std=c++23"
    end
end

function handleDefaultLibConfiguration()
    kind "StaticLib"

    filter "configurations:Debug"
        -- kind "SharedLib"
        defines { "TK_DEBUG" }
    symbols "On"

        runIfOS("windows", function ()
            targetextension ".dll"
        end)

    filter "configurations:Release"
        -- kind "StaticLib"
        defines { "TK_NDEBUG", "TK_RELEASE" }
        symbols "Off"
        optimize "On"

        runIfOS("windows", function ()
            targetextension ".dll"
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
    if os.host() == requiredOS then
        fn()
    end
end

function runIfDisplayServerLinux(displayServer, fn)
    if os.host() ~= "linux" then
        print("Trying to run runIfDisplayServerLinux function on a non linux host")
        os.exit(1)
    end
    
    if LINUX_DISPLAY_SERVER == displayServer then
        fn()
    end
end
