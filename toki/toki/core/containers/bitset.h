#pragma once

#include <toki/core/common/optional.h>
#include <toki/core/types.h>

namespace toki {

template <u64 N>
class Bitset {
	using ByteChunk = u8;

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

	constexpr void flip(u64 index) {
		return set_value(index, !read_value(index));
	}

	constexpr toki::Optional<u64> get_first_with_value(b8 value) const {
		for (u32 i = 0; i < (N - 1) / sizeof(ByteChunk) + 1; i++) {
			if (m_bits[i] & (value ? ~static_cast<u8>(0) : 0)) {
				continue;
			}

			for (u32 shift = 0; shift < sizeof(ByteChunk) * 8; shift++) {
				if (((m_bits[i] >> shift) & 1) == value) {
					return i * sizeof(ByteChunk) + shift;
				}
			}
		}

		return NullOpt{};
	}

private:
	inline constexpr b8 read_value(u64 index) const {
		return (m_bits[index >> 3] >> (~sizeof(ByteChunk) & index)) & 1;
	}

	inline constexpr void set_value(u64 index, b8 value) {
		auto inde = index >> 3;
		auto subindex = (index & (sizeof(ByteChunk) * 8 - 1));
		m_bits[index >> 3] |= ((value & 1) << (index & (sizeof(ByteChunk) * 8 - 1)));
	}

	ByteChunk m_bits[(N - 1) / sizeof(ByteChunk) + 1]{};
};

}  // namespace toki
