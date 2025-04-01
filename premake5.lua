require "premake/export-compile-commands/export-compile-commands"
require "premake.common"
local toki_projects = require "premake.projects"

workspace "Toki"
configurations { "Debug", "Release", "Dist" }
architecture "x64"

if os.host() == "windows" then
    platforms { "Windows" }
elseif os.host() == "linux" then
    platforms { "Linux" }
end

-- startproject "Sandbox"

location "build"

local function MapDeps(proj)
    local links_out = {}
    local includes_out = {}

    if proj.deps == nil or type(proj.deps) ~= "table" then
        return { links = links_out, includedirs = includes_out }
    end

    for _, dep in pairs(proj.deps) do
        table.insert(links_out, toki_projects.libraries[dep].name)
        table.insert(includes_out, path.join(toki_projects.libraries[dep].dir, "include"))
    end

    if proj.extra_lib_includes ~= nil then
        for _, inc in pairs(proj.extra_lib_includes) do
            table.insert(includes_out, path.join(toki_projects.libraries[inc].dir, "include"))
        end
    end

    return { links = links_out, includedirs = includes_out }
end

group "Libraries"
for _, proj in pairs(toki_projects.libraries) do
    ProjectLibrary(proj)

    -- if proj.pch ~= nil then
    --     PrecompiledHeader(proj.pch, proj.dir)
    -- end

    local deps = MapDeps(proj)
    links(deps.links)
    includedirs(deps.includedirs)
end

group "Executables"
for _, proj in pairs(toki_projects.executables) do
    ProjectExecutable(proj)

    local deps = MapDeps(proj)
    links(deps.links)
    includedirs(deps.includedirs)
end
