#pragma once

#include <toki/core/types.h>

namespace toki {

enum ModBits : u16 {
	MOD_BITS_LSHIFT = 1 << 0,
	MOD_BITS_RSHIFT = 1 << 1,
	MOD_BITS_LCONTROL = 1 << 2,
	MOD_BITS_RCONTROL = 1 << 3,
	MOD_BITS_LALT = 1 << 4,
	MOD_BITS_RALT = 1 << 5,
	MOD_BITS_LSUPER = 1 << 6,
	MOD_BITS_RSUPER = 1 << 7,
	// MOD_BITS_CAPS_LOCK = 1 << 8,
	// MOD_BITS_NUM_LOCK = 1 << 9,
	// MOD_BITS_SCROLL_LOCK = 1 << 10,
};

struct Mods {
public:
	b8 has(u8 mask);
	b8 shift();
	b8 control();
	b8 alt();
	b8 super();
	// b8 caps_lock();
	// b8 num_lock();
	// b8 scroll_lock();

public:
	u16 mods{};
};

inline b8 Mods::has(u8 mask) {
	return mods & mask;
}

inline b8 Mods::shift() {
	return mods & (ModBits::MOD_BITS_LSHIFT | ModBits::MOD_BITS_RSHIFT);
}

inline b8 Mods::control() {
	return mods & (ModBits::MOD_BITS_LCONTROL | ModBits::MOD_BITS_RCONTROL);
}

inline b8 Mods::alt() {
	return mods & (ModBits::MOD_BITS_LALT | ModBits::MOD_BITS_RALT);
}

inline b8 Mods::super() {
	return mods & (ModBits::MOD_BITS_LSUPER | ModBits::MOD_BITS_RSUPER);
}

// inline b8 Mods::caps_lock() {
// 	return mods & (ModBits::MOD_BITS_CAPS_LOCK);
// }
//
// inline b8 Mods::num_lock() {
// 	return mods & (ModBits::MOD_BITS_NUM_LOCK);
// }
//
// inline b8 Mods::scroll_lock() {
// 	return mods & (ModBits::MOD_BITS_SCROLL_LOCK);
// }

}  // namespace toki
