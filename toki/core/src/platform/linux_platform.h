#pragma once

#include "core/types.h"

namespace toki {

namespace pt {

// Syscalls
constexpr u64 SYS_READ = 0;
constexpr u64 SYS_WRITE = 1;
constexpr u64 SYS_OPEN = 2;
constexpr u64 SYS_CLOSE = 3;
constexpr u64 SYS_MMAP = 9;
constexpr u64 SYS_MUNMAP = 11;
constexpr u64 SYS_EXIT = 60;
constexpr u64 SYS_CLOCK_GETTIME = 228;

constexpr u64 FILE_CREAT = 0100;
constexpr u64 FILE_TRUNC = 01000;
constexpr u64 FILE_APPEND = 02000;

// Protections
constexpr u64 PROT_NONE = 0x0;
constexpr u64 PROT_READ = 0x1;
constexpr u64 PROT_WRITE = 0x2;
constexpr u64 PROT_RDWR = PROT_READ | PROT_WRITE;

// Sharing types
enum SharingTypes {
	MAP_PRIVATE = 0x02,
	MAP_ANONYMOUS = 0x20
};

}  // namespace pt

}  // namespace toki
