#include "compile_shader_job.h"

#include "core/core.h"
#include "resources/loaders/shader_loader.h"

namespace Toki {

Ref<CompileShaderJob> CompileShaderJob::create(std::filesystem::path path) {
    return createRef<CompileShaderJob>(path);
}

CompileShaderJob::CompileShaderJob(std::filesystem::path path) : path(path) {}

CompileShaderJob::~CompileShaderJob() {}

void CompileShaderJob::execute() {
    std::cout << "started compile\n";
    data = ShaderLoader::loadShader(path);

    std::cout << "ended compile\n";
}

}  // namespace Toki
