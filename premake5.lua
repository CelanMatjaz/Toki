require "premake/export-compile-commands/export-compile-commands"
require "premake/common_premake"

workspace "Toki"
    configurations { "Debug", "Release", "Dist" }
    architecture "x64"
    platforms { "Windows", "Linux" }

startproject "Sandbox"

group "Dependencies"
    include "./vendor"

group "Toki"
    include "./toki_engine"

group "Misc"
    include "./toki_sandbox"

-- group "Examples"
--     include "./examples"
