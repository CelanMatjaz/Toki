#pragma once

#include <toki/core/types.h>

namespace toki {

constexpr u64 STD_IN  = 0;
constexpr u64 STD_OUT = 1;
constexpr u64 STD_ERR = 2;

constexpr const char* TRUE_STR	= "true";
constexpr const char* FALSE_STR = "false";

constexpr const u64 U64_MIN = static_cast<u64>(0);
constexpr const u64 U64_MAX = static_cast<u64>(-1);

constexpr const i64 I64_MAX = static_cast<i64>(static_cast<u64>(static_cast<u64>(1) << 63) - 1);
constexpr const i64 I64_MIN = static_cast<i64>(static_cast<u64>(1) << 63);

constexpr const u32 U32_MIN = static_cast<u32>(0);
constexpr const u32 U32_MAX = static_cast<u32>(-1);

constexpr const i32 I32_MAX = static_cast<i32>(static_cast<u32>(static_cast<u32>(1) << 31) - 1);
constexpr const i32 I32_MIN = static_cast<i32>(static_cast<u32>(1) << 31);

constexpr const u16 U16_MIN = static_cast<u16>(0);
constexpr const u16 U16_MAX = static_cast<u16>(-1);

constexpr const i16 I16_MAX = static_cast<i16>(static_cast<u16>(static_cast<u16>(1) << 15) - 1);
constexpr const i16 I16_MIN = static_cast<i16>(static_cast<u16>(1) << 15);

constexpr const u8 U8_MIN = static_cast<u8>(0);
constexpr const u8 U8_MAX = static_cast<u8>(-1);

constexpr const i8 I8_MAX = static_cast<i8>(static_cast<u8>(static_cast<u8>(1) << 7) - 1);
constexpr const i8 I8_MIN = static_cast<i8>(static_cast<u8>(1) << 7);

}  // namespace toki
