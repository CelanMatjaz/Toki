# Libs
set(CORE_TARGET_NAME Core)
set(ENGINE_TARGET_NAME Engine)
set(RENDERER_TARGET_NAME Renderer)

# Executables
set(SANDBOX_TARGET_NAME Sandbox)

add_subdirectory(core)
add_subdirectory(engine)
add_subdirectory(renderer)

add_subdirectory(sandbox)
