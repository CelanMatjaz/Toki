require "premake/export-compile-commands/export-compile-commands"
require "premake/common_premake"

workspace "Toki"
    configurations { "Debug", "Release", "Dist" }
    architecture "x64"
    platforms { "Windows", "Linux" }

startproject "Sandbox"

group "Dependencies"
    -- include "./vendor"

group "Toki"
    include "./toki_libs"

group "Misc"
    include "./toki_executables"

-- group "Examples"
--     include "./toki_examples"
