#pragma once

#include <toki/core/types.h>

namespace toki {

enum ModBits : u8 {
	LSHIFT = 1 << 0,
	RSHIFT = 1 << 1,
	LCONTROL = 1 << 2,
	RCONTROL = 1 << 3,
	LALT = 1 << 4,
	RALT = 1 << 5,
	LSUPER = 1 << 6,
	RSUPER = 1 << 7
};

class Mod {
public:
	b8 has(u8 mask);
	b8 shift();
	b8 control();
	b8 alt();
	b8 super();

public:
	u8 mod{};
};

inline b8 Mod::has(u8 mask) {
	return mod & mask;
}

inline b8 Mod::shift() {
	return mod & (ModBits::LSHIFT | ModBits::RSHIFT);
}

inline b8 Mod::control() {
	return mod & (ModBits::LCONTROL | ModBits::RCONTROL);
}

inline b8 Mod::alt() {
	return mod & (ModBits::LALT | ModBits::RALT);
}

inline b8 Mod::super() {
	return mod & (ModBits::LSUPER | ModBits::RSUPER);
}

}  // namespace toki::platform
