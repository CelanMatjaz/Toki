#pragma once

#include "../types/types.h"
#include "../utils/macros.h"

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
    Time();
    Time(u64 time);

    template <Unit Unit = Unit::Seconds>
    f32 as();

    Time operator+(Time other);
    Time operator-(Time other);

private:
    // Time in nanoseconds
    u64 _time{};
};

}  // namespace toki
