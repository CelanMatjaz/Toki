#pragma once

#include <toki/core/core.h>
#include <toki/core/errors.h>

namespace toki {

enum Syscalls : i64 {
	SYS_READ   = 0,
	SYS_WRITE  = 1,
	SYS_OPEN   = 2,
	SYS_CLOSE  = 3,
	SYS_LSEEK  = 8,
	SYS_MMAP   = 9,
	SYS_MUNMAP = 11,
	SYS_CLONE  = 56,
	SYS_FUTEX  = 202,
};

}  // namespace toki
