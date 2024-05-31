#pragma once

#include <expected>
#include <vector>

#include "toki/core/errors.h"
#include "toki/renderer/renderer_types.h"
#include "toki/resources/serializers/shader_serializer.h"

namespace Toki {

class ShaderCompiler {
public:
    static std::expected<std::vector<uint32_t>, Error> compileShader(const ShaderSource& source, ShaderStage stage);

private:
};

}  // namespace Toki
