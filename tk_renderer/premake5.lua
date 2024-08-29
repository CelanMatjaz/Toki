project "TK_Renderer"
    handleCppDialect()
    targetAndObjectDirs()

    files { "src/**.h", "src/**.cpp" }
    links { "TK_Core", "GLFW" }
    includedirs {
        "%{wks.location}/tk_renderer/src",
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
