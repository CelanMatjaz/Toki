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

using byte = unsigned char;
using word = wchar_t;

template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr Scope<T> create_scope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> create_ref(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

}  // namespace toki
