#pragma once

#include <toki/core/common/optional.h>
#include <toki/core/types.h>

namespace toki {

template <u64 N>
	requires(N > 0)
class Bitset {
	// NOTE(Matjaž): Changing the chunk type to a type with greater bit count that 8 will break this container!
	using ByteChunk = u8;
	static constexpr u64 BYTE_CHUNK_COUNT = (N - 1) / (sizeof(ByteChunk) * 8) + 1;
	static constexpr u64 BYTE_CHUNK_BIT_COUNT = sizeof(ByteChunk) * 8;

public:
	constexpr Bitset() = default;

	constexpr inline void set(u64 index, b8 value) {
		return set_value(index, value);
	}

	constexpr inline b8 operator[](u64 index) const {
		return at(index);
	}

	constexpr inline b8 at(u64 index) const {
		return read_value(index);
	}

	// Flip every bit
	constexpr void flip() {
		for (u32 i = 0; i < BYTE_CHUNK_COUNT; i++) {
			m_bits[i] = ~m_bits[i];
		}
	}

	constexpr void flip(u64 index) {
		m_bits[index >> 3] ^= (1 << (index & (8 - 1)));
	}

	constexpr u64 size() const {
		return N;
	}

	constexpr toki::Optional<u64> get_first_with_value(b8 value) const {
		return get_first_with_value_from(0, value);
	}

	constexpr toki::Optional<u64> get_first_with_value_from(u64 index, b8 value) const {
		u32 i = index >> 3;
		u32 shift = index & 0b111;
		for (; i < BYTE_CHUNK_COUNT; i++) {
			if ((m_bits[i] == (value ? 0 : ~(static_cast<u8>(0))))) {
				continue;
			}

			for (; shift < BYTE_CHUNK_BIT_COUNT; shift++) {
				if (((m_bits[i] >> shift) & 1) == value) {
					return i * sizeof(ByteChunk) * 8 + shift;
				}
			}
		}

		return NullOpt{};
	}

private:
	inline constexpr b8 read_value(u64 index) const {
		return (m_bits[index >> 3] >> (index & 0b111)) & 1;
	}

	inline constexpr void set_value(u64 index, b8 value) {
		m_bits[index >> 3] = (m_bits[index >> 3] & ~(1 << (index & 0b111))) | ((value ? 1 : 0) << (index & 0b111));
	}

	ByteChunk m_bits[BYTE_CHUNK_COUNT]{};
};

}  // namespace toki
