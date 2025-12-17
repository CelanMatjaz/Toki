#pragma once

#include <toki/core/common/common.h>

namespace toki {

template <typename Callable, typename... Args>
void invoke(Callable&& f, Args&&... args) {
	toki::forward<Callable>(f)(toki::forward<Args>(args)...);
}

}  // namespace toki
