#pragma once

#include <cstdint>

namespace Toki {

enum class Error : uint64_t {
    NO_ERROR = 0,

    // Renderer
    RENDERER_INIT_ERROR,
    RENDERER_DEVICE_PROPERTY_ERROR,
    RENDERER_CREATE_DEVICE_ERROR,
    RENDERER_CREATE_SURFACE_ERROR,
    RENDERER_CREATE_SWAPCHAIN_ERROR,
    RENDERER_CREATE_FRAME_ERROR,
    RENDERER_CREATE_RESOURCE_ERROR,

    TK_ERROR_COUNT
};

struct TkError {
    Error error = Error::NO_ERROR;
    uint64_t code{};

    operator bool() { return error == Error::NO_ERROR; }
};

#define ASSERT_ERROR(e, msg, ...)                 \
    if (TkError error = e; !error) {                                     \
        std::println(msg __VA_OPT__(, ) __VA_ARGS__); \
        return error;                                 \
    }

}  // namespace Toki
