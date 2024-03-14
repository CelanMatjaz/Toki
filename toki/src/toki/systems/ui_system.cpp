#include "ui_system.h"

#include "toki/core/assert.h"
#include "toki/core/core.h"

namespace Toki {

struct UISystemState {
    Application* application = nullptr;
};

static Scope<UISystemState> statePtr;

void UISystem::init(Application* application) {
    TK_ASSERT(application != nullptr, "Cannot init UISystem without a provided Application pointer");

    statePtr = createScope<UISystemState>();
    statePtr->application = application;
}

void UISystem::shutdown() {}

}  // namespace Toki
