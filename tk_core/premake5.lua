project "TK_Core"
    handleCppDialect()
    targetAndObjectDirs()

    files { "src/**.h", "src/**.cpp" }
    includedirs { 
        "%{wks.location}/tk_core/src"
    }

    handleDefaultLibConfiguration()
