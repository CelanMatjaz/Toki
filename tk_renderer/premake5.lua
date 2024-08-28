project "TK_Renderer"
    handleCppDialect()
    targetAndObjectDirs()

    files { "src/**.h", "src/**.cpp" }

    if os.host() == "windows" then
        includedirs { VULKAN_SDK .. "/Include" }
        libdirs { VULKAN_SDK .. "/Lib" }
        links { "vulkan-1" }
    elseif os.host() == "linux" then 
        includedirs { VULKAN_SDK .. "/include" }
        libdirs { VULKAN_SDK .. "/lib" }
        links { "vulkan" }
    end

    handleDefaultLibConfiguration()
