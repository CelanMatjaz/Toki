#include "assert.h"

#include <cstdio>

#ifndef TK_DIST

#include <chrono>
#include <print>

void debug_break() {
#if defined(_MSC_VER)
    __debugbreak();
#else
    __builtin_trap();
#endif
}

static const std::chrono::time_zone* currentZone = std::chrono::get_tzdb().current_zone();

void print_assert_error(auto val, const char* file, const char* line) {
    std::println(stderr, "Toki assertion error: {}", val);
}

#endif
