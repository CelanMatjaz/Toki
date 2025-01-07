#pragma once

#include <cstdint>
#include <memory>
#include <utility>

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
using word = wchar_t;
static_assert(sizeof(word) == 2, "Type word does not contain 16 bits");

template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr Scope<T> create_scope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr ref<T> create_ref(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

using handle = u64;

}  // namespace toki
