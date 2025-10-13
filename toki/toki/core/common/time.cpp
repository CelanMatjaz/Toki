#include <toki/core/common/time.h>
#include <toki/platform/syscalls.h>

namespace toki {

Time Time::now() {
	return {};
}

#define TIME_AS(precision, divider)                    \
	template <>                                        \
	u64 Time::as<TimePrecision::precision>() const {   \
		return platform::get_current_time() / divider; \
	}

TIME_AS(Nanos, 1);
TIME_AS(Micros, 1'000);
TIME_AS(Millis, 1'000'000);
TIME_AS(Seconds, 1'000'000'000);

}  // namespace toki
