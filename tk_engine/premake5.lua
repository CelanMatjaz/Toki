project "TK_Engine"
    handleCppDialect()
    targetAndObjectDirs()

    files { "src/**.h", "src/**.cpp" }
    links { "TK_Renderer", "TK_Core" }
    includedirs {
        "%{wks.location}/tk_engine/src",
        "%{wks.location}/tk_renderer/include",
        "%{wks.location}/tk_core/include",
        "%{wks.location}/vendor/glfw/include",
    }

    runIfOS("windows", function ()
        includedirs { path.normalize(path.join(VULKAN_SDK, "Include")) }
        libdirs { path.normalize(path.join(VULKAN_SDK, "Lib")) }
        links { "vulkan-1" }
    end)

    runIfOS("linux", function ()
        includedirs { path.normalize(path.join(VULKAN_SDK, "include")) }
        libdirs { path.normalize(path.join(VULKAN_SDK, "lib")) }
        links { "vulkan" }
    end)

    handleDefaultLibConfiguration()
