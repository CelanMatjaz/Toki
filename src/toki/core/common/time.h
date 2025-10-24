#pragma once

#include <X11/X.h>
#include <toki/core/types.h>

namespace toki {

enum class TimePrecision {
	Seconds,
	Millis,
	Micros,
	Nanos
};

struct Time {
public:
	Time() = default;
	Time(u64 time): m_time(time) {}

	static Time now();

	Time operator-(const Time& other) {
		return { m_time - other.m_time };
	}

	template <TimePrecision>
	f64 as() const;

private:
	// Time in microseconds since epoch
	u64 m_time{};
};

}  // namespace toki
