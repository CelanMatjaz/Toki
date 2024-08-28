require "utils"

project "GLFW"
    kind "StaticLib"
    language "C"
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

    files {
        "glfw/src/**.h",
        "glfw/src/**.c"
    }
    includedirs { 
        "glfw/include",
        "includes/glfw"
    }

    print ("dwjaiodawjiodjwaojdi")
    print (LINUX_DISPLAY_SERVER)

if LINUX_DISPLAY_SERVER == "wayland" then
    generateWaylandProtocolFiles()
    defines { "_GLFW_WAYLAND" }
elseif LLINUX_DISPLAY_SERVER == "x11" then
    defines { "_GLFW_X11" }
end

