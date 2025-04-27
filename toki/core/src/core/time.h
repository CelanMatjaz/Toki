#pragma once

#include "../platform/platform.h"
#include "types.h"

namespace toki {

class Time {
public:
	enum class Unit {
		Seconds = 0,
		Milliseconds,
		Microseconds,
		Nanoseconds,
	};

public:
	Time(): m_time(pt::time_nanoseconds()) {}
	Time(u64 time): m_time(time) {}

	static Time now() {
		return Time();
	}

	template <Time::Unit Unit>
	f32 as() {
		if constexpr (Unit == Time::Unit::Nanoseconds) {
			return m_time;
		}

		else if constexpr (Unit == Time::Unit::Microseconds) {
			return m_time / 1'000.0f;
		}

		else if constexpr (Unit == Time::Unit::Milliseconds) {
			return m_time / 1'000'000.0f;
		}

		else if constexpr (Unit == Time::Unit::Seconds) {
			return m_time / 1'000'000'000.0f;
		}

		else {
			static_assert(false, "Unhandled unit type passed as template argument");
		}
	}

	Time operator+(Time other) {
		return Time(m_time + other.m_time);
	}

	Time operator-(Time other) {
		return Time(m_time - other.m_time);
	}

private:
	// Time in nanoseconds
	u64 m_time{};
};

}  // namespace toki
