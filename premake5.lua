require "premake/export-compile-commands/export-compile-commands"
require "premake/common.premake"

workspace "Toki"
    configurations { "debug", "release", "dist" }
    architecture "x64"

if os.host() == "linux" then
    platforms { "linux_wayland", "linux_x11" }
end

if os.host() == "windows" then
    platforms { "windows" }
end

startproject "Sandbox"

group "Dependencies"
    include "./vendor"

group "Toki"
    include "./toki_engine"

group "Misc"
    include "./toki_sandbox"

group "Examples"
    include "./examples"
