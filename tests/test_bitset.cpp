#include "testing.h"
//

#include <toki/core/core.h>

using namespace toki;

constexpr const u32 BITSET_BYTE_COUNT = 16;
constexpr const u32 BITSET_BIT_COUNT = BITSET_BYTE_COUNT * 8;

template <u64 N>
void fill_bitset_with_values(toki::Bitset<N>& bitset, b8 value) {
	for (u32 i = 0; i < bitset.size(); i++) {
		bitset.set(i, value);
	}
}

TK_TEST(Bitset, should_set_correct_values_on_correct_bytes) {
	Bitset<BITSET_BIT_COUNT> bitset;

	for (u32 i = 0; i < BITSET_BYTE_COUNT; i++) {
		for (u32 j = 0; j < 8; j++) {
			bitset.set(i * 8 + j, j % 2);
		}

		TK_TEST_ASSERT(bitset.m_bits[i] == 0b10101010);
	}

	return true;
}

TK_TEST(Bitset, should_flip_every_bit_correctly) {
	Bitset<BITSET_BIT_COUNT> bitset;

	for (u32 i = 0; i < BITSET_BYTE_COUNT; i++) {
		for (u32 j = 0; j < 8; j++) {
			bitset.set(i * 8 + j, j % 2);
		}
	}

	bitset.flip();
	for (u32 i = 0; i < BITSET_BYTE_COUNT; i++) {
		TK_TEST_ASSERT(bitset.m_bits[i] == 0b01010101);
	}

	return true;
}

TK_TEST(Bitset, should_flip_every_bit_on_index_correctly) {
	Bitset<BITSET_BIT_COUNT> bitset;

	for (u32 i = 0; i < BITSET_BYTE_COUNT; i++) {
		for (u32 j = 0; j < 8; j++) {
			bitset.set(i * 8 + j, j % 2);
		}
	}

	for (u32 i = 0; i < BITSET_BYTE_COUNT; i++) {
		for (u32 j = 0; j < 8; j += 2) {
			bitset.flip(i * 8 + j);
		}

		TK_TEST_ASSERT(bitset.m_bits[i] == 0b11111111);
	}

	return true;
}

TK_TEST(Bitset, should_return_correct_bit_index) {
	Bitset<BITSET_BIT_COUNT> bitset;

	fill_bitset_with_values(bitset, false);
	TK_TEST_ASSERT(!bitset.get_first_with_value(true).has_value());
	TK_TEST_ASSERT(bitset.get_first_with_value(false).value() == 0);

	fill_bitset_with_values(bitset, true);
	TK_TEST_ASSERT(!bitset.get_first_with_value(false).has_value());
	TK_TEST_ASSERT(bitset.get_first_with_value(true).value() == 0);

	return true;
}

TK_TEST(Bitset, should_provide_correct_indices) {
	Bitset<BITSET_BIT_COUNT> bitset;

	fill_bitset_with_values(bitset, false);
	TK_TEST_ASSERT(!bitset.get_first_with_value(true).has_value());
	TK_TEST_ASSERT(bitset.get_first_with_value(false).value() == 0);

	fill_bitset_with_values(bitset, true);
	TK_TEST_ASSERT(!bitset.get_first_with_value(false).has_value());
	TK_TEST_ASSERT(bitset.get_first_with_value(true).value() == 0);

	bitset.m_bits[0] = 0b11110000;
	TK_TEST_ASSERT(bitset.get_first_with_value(true).value() == 4);

	fill_bitset_with_values(bitset, true);
	bitset.m_bits[4] = 0b11101111; 
	u64 index = bitset.get_first_with_value(false).value();
	TK_TEST_ASSERT(index == 4 * 8 + 4);

	return true;
}
