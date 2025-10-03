# Libs
set(CORE_TARGET_NAME core)
set(RUNTIME_TARGET_NAME runtime)
set(RENDERER_TARGET_NAME renderer)
set(PLATFORM_TARGET_NAME platform)

# Executables
set(SANDBOX_TARGET_NAME sandbox)

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(runtime)
add_subdirectory(renderer)

add_subdirectory(sandbox)
