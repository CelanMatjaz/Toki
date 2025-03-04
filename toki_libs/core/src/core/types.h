#pragma once

namespace toki {

using b8 = bool;
static_assert(sizeof(b8) == 1);
using b16 = short;
static_assert(sizeof(b16) == 2);
using b32 = int;
static_assert(sizeof(b32) == 4);

using i8 = char;
static_assert(sizeof(i8) == 1);
using u8 = unsigned char;
static_assert(sizeof(u8) == 1);

using i16 = short;
static_assert(sizeof(i16) == 2);
using u16 = unsigned short;
static_assert(sizeof(u16) == 2);

using i32 = int;
static_assert(sizeof(i32) == 4);
using u32 = unsigned int;
static_assert(sizeof(u32) == 4);

using i64 = long long;
static_assert(sizeof(i64) == 8);
using u64 = unsigned long long;
static_assert(sizeof(u64) == 8);

using f32 = float;
static_assert(sizeof(f32) == 4);
using f64 = double;
static_assert(sizeof(f64) == 8);

using byte = unsigned char;
static_assert(sizeof(byte) == 1);
using word = u16;
static_assert(sizeof(word) == 2);
using wchar = u32;
static_assert(sizeof(wchar) == 4);

}  // namespace toki
