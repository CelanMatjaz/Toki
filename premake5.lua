VULKAN_SDK = os.getenv("VULKAN_SDK")

workspace "Workspace"
    language "C++"
    cppdialect "C++20"
    configurations { "Debug", "Release" }
    platforms { "Win64" }
    architecture "x64"

outputdir = path.getabsolute("bin/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.architecture}")

group "Dependencies"
    include "./toki/vendor/imgui"
	include "./toki/vendor/glfw"
group ""

group "Engine"
    include "Toki"
group ""

