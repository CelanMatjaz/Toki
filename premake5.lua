require "premake/export-compile-commands/export-compile-commands"
require "premake/common.premake"

workspace "Toki"
    configurations { "debug", "release", "dist" }
    architecture "x64"

    startproject "Sandbox"

group "Dependencies"
    include "./vendor"

group "Toki"
    include "./toki_core"
    include "./toki_renderer"
    include "./toki_engine"

group "Misc"
    include "./toki_sandbox"
