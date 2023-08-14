include("premake.paths.lua")

workspace "Workspace"
    language "C++"
    cppdialect "C++20"
    configurations { "Debug", "Release" }
    platforms { "Win64" }
    architecture "x64"

startproject "Toki"

group "Engine"
    include "Toki"
group ""

group "Dependencies"
    include "./toki/vendor/imgui"
	include "./toki/vendor/glfw
group ""

