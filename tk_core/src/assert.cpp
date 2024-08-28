#include "assert.h"

#include <cstdio>

#ifndef TK_DIST

#include <chrono>
#include <print>

void debugBreak() {
    __builtin_trap();
}

static const std::chrono::time_zone* currentZone = std::chrono::get_tzdb().current_zone();

void printAssertError(auto val, const char* file, const char* line) {
    std::println(stderr, "Toki assertion error: {}", val);
}

#endif
