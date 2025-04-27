#include <sys/time.h>

#include <ctime>

#include "../time.h"

namespace toki {

static timespec get_time() {
    timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return tp;
}

Time::Time(): _time(get_time().tv_nsec) {}

Time::Time(u64 time): _time(time) {}

template <Time::Unit Unit>
f32 Time::as() {
    if constexpr (Unit == Time::Unit::Nanoseconds) {
        return _time;
    }

    else if constexpr (Unit == Time::Unit::Microseconds) {
        return _time / 1'000.0;
    }

    else if constexpr (Unit == Time::Unit::Milliseconds) {
        return _time / 1'000'000.0;
    }

    else if constexpr (Unit == Time::Unit::Seconds) {
        return _time / 1'000'000'000.0;
    }

    else {
        static_assert(false, "Unhandled unit type passed as template argument");
    }
}

Time Time::operator+(Time other) {
    return { _time + other._time };
}

Time Time::operator-(Time other) {
    return { _time - other._time };
}

}  // namespace toki
