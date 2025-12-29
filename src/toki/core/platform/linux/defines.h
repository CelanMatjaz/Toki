#pragma once

#include <toki/core/types.h>

namespace toki {

enum Syscalls : u64 {
	READ		  = 0,
	WRITE		  = 1,
	OPEN		  = 2,
	CLOSE		  = 3,
	LSEEK		  = 8,
	MMAP		  = 9,
	MUNMAP		  = 11,
	NANOSLEEP	  = 35,
	CLONE		  = 56,
	FUTEX		  = 202,
	CLOCK_GETTIME = 224,
};

enum Clock : u64 {
	CLOCK_REALTIME = 0,
};

}  // namespace toki
