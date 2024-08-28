project "TK_Engine"
    handleCppDialect()
    targetAndObjectDirs()

    files { "src/**.h", "src/**.cpp" }
    links { "TK_Renderer", "TK_Core" }
    includedirs {
        "%{wks.location}/tk_renderer/include",
        "%{wks.location}/tk_core/include",
    }

    handleDefaultLibConfiguration()
