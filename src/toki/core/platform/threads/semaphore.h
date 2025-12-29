#pragma once

#include <toki/core/types.h>

namespace toki {

class Semaphore {
public:
	Semaphore(u32 count);

	void acquire();
	void release();

private:
	i32 m_count{};
};

}  // namespace toki
