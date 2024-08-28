require "./premake/export-compile-commands/export-compile-commands"


if os.host() == "linux" then
    LINUX_DISPLAY_SERVER = os.getenv("XDG_SESSION_TYPE")
end

workspace "Toki"
    configurations { "Debug", "Release" }
    architecture "x86_64"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Core"
    include "./tk_sandbox"
    include "./tk_core"
    include "./tk_renderer"
group ""

group "Dependendies"
    include "./vendor"
group ""
