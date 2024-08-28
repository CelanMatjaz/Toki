project "TK_Core"
    handleCppDialect()
    targetAndObjectDirs()

    files { "src/**.h", "src/**.cpp" }
    links { "TK_Renderer" }
    includedirs { "%{wks.location}/tk_renderer/src" }

    handleDefaultLibConfiguration()
