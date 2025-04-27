#pragma once

#include "../core/types.h"

namespace toki {

class Allocator;

namespace pt {

void initialize(Allocator& allocator);

void shutdown(Allocator& allocator);

constexpr u64 STD_IN = 0;
constexpr u64 STD_OUT = 1;
constexpr u64 STD_ERR = 2;

struct Handle {
	Handle(u64 value): handle(static_cast<i64>(value)) {}
	i64 handle;
};

extern u64 last_error;

inline u64 get_last_error() {
	return last_error;
}

i64 exit(i32 error);

i64 read(Handle handle, char* buf, u64 count);

i64 write(Handle handle, const char* buf, u64 count);

i64 open(const char* pathname, i32 flags);

i64 close(Handle handle);

void* allocate(u64 size);

void deallocate(void* ptr);

u64 time_nanoseconds();

}  // namespace pt

}  // namespace toki
