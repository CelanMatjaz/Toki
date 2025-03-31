#pragma once

#include <toki/core.h>
#include <toki/renderer.h>

namespace toki {

struct CompileShaderConfig {
    BumpAllocator& allocator;
    ShaderStage stage;
    char* source_path;
};

#if defined(_WIN32)

#if defined(TK_SHARED_LIB_EXPORT)
#define SHADER_COMPILER_API __declspec(dllexport)
#else
#define SHADER_COMPILER_API __declspec(dllimport)
#endif

#endif

#ifndef SHADER_COMPILER_API
#define SHADER_COMPILER_API
#endif

SHADER_COMPILER_API BumpRef<u32> compile_shader(const CompileShaderConfig& config);

}  // namespace toki
