#include <toki/core/common/time.h>
#include <toki/core/platform/syscalls.h>

namespace toki {

Time Time::now() {
	auto result = get_current_time();
	return result.value_or(0);
}

#define TIME_AS(precision, divider)                  \
	template <>                                      \
	f64 Time::as<TimePrecision::precision>() const { \
		return m_time / divider;                     \
	}

TIME_AS(Nanos, 1.0);
TIME_AS(Micros, 1'000.0);
TIME_AS(Millis, 1'000'000.0);
TIME_AS(Seconds, 1'000'000'000.0);

}  // namespace toki
