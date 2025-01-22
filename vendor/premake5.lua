project "GLFW"
    language "C++"
    kind "StaticLib"
    files { "glfw/src/**.c", "glfw/src/**.h" }
    includedirs { "glfw/src" }

    defines { "_GLFW_VULKAN_STATIC" }

    set_target_and_object_dirs()
    configuration_configs_libs()

    filter "platforms:Windows"
        defines { "_GLFW_WIN32" }
        links { "gdi32", "user32", "shell32" }

    filter "platforms:Linux"
        defines { "_GLFW_WAYLAND ", "_GLFW_X11" }
        includedirs { "includes/glfw" }

project "yaml-cpp"
    language "C++"
    kind "StaticLib"
    files { "yaml-cpp/src/**.cpp", "yaml-cpp/src/**.h" }
    includedirs { "yaml-cpp/src", "yaml-cpp/include" }

    defines { "YAML_CPP_STATIC_DEFINE", "YAML_CPP_API=" }

    set_target_and_object_dirs()
    configuration_configs_libs()

project "freetype"
    language "C"
    kind "StaticLib"
    files { -- Sources copied from CMakeLists.txt (BASE_SRCS variable)
        "free_type/src/autofit/autofit.c",
        "free_type/src/base/ftbase.c",
        "free_type/src/base/ftbbox.c",
        "free_type/src/base/ftbdf.c",
        "free_type/src/base/ftbitmap.c",
        "free_type/src/base/ftcid.c",
        "free_type/src/base/ftdebug.c",
        "free_type/src/base/ftfstype.c",
        "free_type/src/base/ftgasp.c",
        "free_type/src/base/ftglyph.c",
        "free_type/src/base/ftgxval.c",
        "free_type/src/base/ftinit.c",
        "free_type/src/base/ftmm.c",
        "free_type/src/base/ftotval.c",
        "free_type/src/base/ftpatent.c",
        "free_type/src/base/ftpfr.c",
        "free_type/src/base/ftstroke.c",
        "free_type/src/base/ftsynth.c",
        "free_type/src/base/fttype1.c",
        "free_type/src/base/ftwinfnt.c",
        "free_type/src/bdf/bdf.c",
        "free_type/src/bzip2/ftbzip2.c",
        "free_type/src/cache/ftcache.c",
        "free_type/src/cff/cff.c",
        "free_type/src/cid/type1cid.c",
        "free_type/src/gzip/ftgzip.c",
        "free_type/src/lzw/ftlzw.c",
        "free_type/src/pcf/pcf.c",
        "free_type/src/pfr/pfr.c",
        "free_type/src/psaux/psaux.c",
        "free_type/src/pshinter/pshinter.c",
        "free_type/src/psnames/psnames.c",
        "free_type/src/raster/raster.c",
        "free_type/src/sdf/sdf.c",
        "free_type/src/sfnt/sfnt.c",
        "free_type/src/smooth/smooth.c",
        "free_type/src/svg/svg.c",
        "free_type/src/truetype/truetype.c",
        "free_type/src/type1/type1.c",
        "free_type/src/type42/type42.c",
        "free_type/src/winfonts/winfnt.c",
    }
    includedirs { "free_type/src", "free_type/include" }

    defines { "FT2_BUILD_LIBRARY", "WIN32" }

    set_target_and_object_dirs()
    configuration_configs_libs()

    filter "platforms:Windows"
        files {
            "free_type/builds/windows/ftsystem.c",
            "free_type/builds/windows/ftdebug.c",
        }
        defines { "WIN32", "WIN32_LEAN_AND_MEAN" }

    filter "platforms:Linux"
        files {
            "free_type/builds/unix/ftsystem.c",
        }
        includedirs { "free_type/src/builds/unix/ftconfig.h.in" }
        defines { "HAVE_FCNTL_H=", "HAVE_UNISTD_H=" }

    filter {}
