VULKAN_SDK = os.getenv("VULKAN_SDK")

workspace "Workspace"
    language "C++"
    cppdialect "C++20"
    configurations { "Debug", "Release" }
    platforms { "Win64" }
    architecture "x64"

outputdir = path.getabsolute("bin/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.architecture}")
outputdirObj = path.getabsolute("bin-int/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.architecture}")

startproject "Toki"

group "Engine"
    include "Toki"
group ""

group "Dependencies"
    include "./toki/vendor/imgui"
	include "./toki/vendor/glfw"
group ""

