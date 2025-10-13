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

	template <TimePrecision>
	u64 as() const;

private:
	// Time in microseconds since epoch
	u64 m_time{};
};

}  // namespace toki
