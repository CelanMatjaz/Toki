projectRoot = path.getabsolute(".")

outputDir = "bin/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.architecture}"
outputDirObj = "bin-int/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.architecture}"

solutionOutputDir = path.join(projectRoot, outputDir)

projectOutputDir = path.join(projectRoot, "bin/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.architecture}/%{prj.name}")
projectOutputDirObj = path.join(projectRoot, "bin-int/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.architecture}/%{prj.name}")

tokiDir = path.join(projectRoot, "Toki")

tokiVendorDir = path.join(tokiDir, "vendor")
tokiVendorGLFW = path.join(tokiVendorDir, "GLFW")
tokiVendorGLM = path.join(tokiVendorDir, "GLM")
tokiVendorImGui = path.join(tokiVendorDir, "ImGui")
tokiVendorSPIRVCross = path.join(tokiVendorDir, "SPIRV-Cross")
tokiVendorSTB = path.join(tokiVendorDir, "stb")

VULKAN_SDK = os.getenv("VULKAN_SDK")

-- includes
includes = {}
includes["vulkan"] = path.join(VULKAN_SDK, "Include")
includes["glfw"] = path.join(tokiVendorGLFW, "include")
includes["glm"] = tokiVendorGLM
includes["imgui"] = tokiVendorImGui
includes["imgui-backends"] = path.join(tokiVendorImGui, "backends")
includes["spirv-cross"] = path.join(tokiVendorSPIRVCross, "src")
includes["stb"] = tokiVendorSTB

includes["toki"] = path.join(tokiDir, "src")

-- link dirs
linkDirs = {}
linkDirs["vulkan"] = path.join(VULKAN_SDK, "Lib")
linkDirs["glfw"] = path.join(tokiVendorGLFW, outputDir)
linkDirs["imgui"] = path.join(tokiVendorImGui, outputDir)
linkDirs["spirv-cross"] = path.join(tokiVendorSPIRVCross, outputDir)

-- main project link dirs
linkDirs["toki"] = path.join(solutionOutputDir, "Toki")

-- links
linkLibs = {}
linkLibs["vulkan"] = "vulkan-1"
linkLibs["gdi32"] = "gdi32"
linkLibs["glfw"] = "GLFW"
linkLibs["imgui"] = "ImGui"
linkLibs["spirv-cross"] = "SPIRV-Cross"

-- main project links
linkLibs["toki"] = "Toki"