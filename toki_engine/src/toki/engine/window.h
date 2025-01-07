#pragma once

#include <memory>
#include <string>

#include "core/core.h"
#include "math/types.h"
#include "math/vector.h"

namespace toki {

class window {
public:
    struct config {
        i32 width = 0;
        i32 height = 0;
        std::string title;
    };

public:
    static ref<window> create(const config& config);

    window() = delete;
    window(const config& config);
    virtual ~window() = 0;

    virtual Vec2i get_dimensions() const = 0;

    virtual bool should_close() const = 0;
    virtual void* get_handle() const = 0;
    static void poll_events();
};

}  // namespace toki
