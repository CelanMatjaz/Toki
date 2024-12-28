require "premake/export-compile-commands/export-compile-commands"
require "premake/common.premake"

workspace "Toki"
    configurations { "debug", "release", "dist" }
    platforms { "linux_wayland", "linux_x11", "windows" }
    architecture "x64"

startproject "Sandbox"

group "Dependencies"
    include "./vendor"

group "Toki"
    include "./toki_engine"

group "Misc"
    include "./toki_sandbox"
