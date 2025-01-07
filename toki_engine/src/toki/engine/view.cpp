#include "view.h"

namespace toki {

ref<view> view::create() {
    return std::make_shared<view>();
}

}  // namespace toki
