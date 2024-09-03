require "./vendor/export-compile-commands/export-compile-commands"
require "common.premake"

workspace "Toki"
    configurations { "Debug", "Release", "Dist" }
    architecture "x86_64"

startproject "TK_Sandbox"

group "Core"
    include "./tk_core"
    include "./tk_engine"
    include "./tk_renderer"
    include "./tk_sandbox"
group ""

group "Dependendies"
    include "./vendor"
group ""
