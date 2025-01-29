#pragma once

#include <cstdint>
#include <ctime>
#include <glm/glm.hpp>

namespace toki {

using b8 = bool;
using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;

using f32 = float;
static_assert(sizeof(f32) == 4, "Type f32 does not contain 32 bits");
using f64 = double;
static_assert(sizeof(f64) == 8, "Type f64 does not contain 64 bits");

using byte = std::byte;
static_assert(sizeof(byte) == 1, "Type byte does not contain 8 bits");
using word = u16;
static_assert(sizeof(word) == 2, "Type word does not contain 16 bits");

constexpr u64 INVALID_HANDLE_ID = 0;

struct Handle {
    Handle(): unique_id(INVALID_HANDLE_ID), index(0), data(0) {}
    Handle(u32 index, u32 data = 0): unique_id(time(0)), index(index), data(data) {}

    operator bool() {
        return unique_id != INVALID_HANDLE_ID;
    }

    u64 unique_id;
    u32 index;
    u32 data;
};

constexpr u64 Kilobytes(auto value) {
    return 1024 * value;
}

constexpr u64 Megabytes(auto value) {
    return 1024 * Kilobytes(value);
}

constexpr u64 Gigabytes(auto value) {
    return 1024 * Megabytes(value);
}

}  // namespace toki
