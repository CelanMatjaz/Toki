project "TK_Renderer"
    handleCppDialect()
    targetAndObjectDirs()

    files { "src/**.h", "src/**.cpp" }
    links { "TK_Core" }
    includedirs {
        "%{wks.location}/tk_core/include",
    }

    runIfOS("windows", function ()
        includedirs { VULKAN_SDK .. "/Include" }
        libdirs { VULKAN_SDK .. "/Lib" }
        links { "vulkan-1" }
    end)

    runIfOS("linux", function ()
        includedirs { VULKAN_SDK .. "/include" }
        libdirs { VULKAN_SDK .. "/lib" }
        links { "vulkan" }
    end)

    handleDefaultLibConfiguration()
