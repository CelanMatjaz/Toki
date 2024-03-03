#pragma once

#include "toki/core/core.h"
#include "toki/core/window.h"

namespace Toki {

class Renderer {
public:
    static Scope<Renderer> create();

    virtual void init() = 0;
    virtual void shutdown() = 0;
};

}  // namespace Toki