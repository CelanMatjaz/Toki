#include "view.h"

namespace toki {

Ref<View> View::create() {
    return std::make_shared<View>();
}

}  // namespace toki
