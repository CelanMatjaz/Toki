#pragma once

#include "toki/events/event.h"

namespace Toki {

class Application;

class UISystem {
    friend Application;

public:
    static void init(Application* application);
    static void shutdown();

private:
    inline static Application* s_application = nullptr;
};

}  // namespace Toki
