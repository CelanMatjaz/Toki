#include <ostream>
#include <print>

#include "containers/dynamic_array.h"
#include "core/base.h"
#include "platform/platform_window.h"

int tk_entry_point(int, char**) {
    toki::platform::Window window = toki::platform::create_window("Test");

    {
        toki::containers::DynamicArray<toki::u64> vec;
        vec.resize(20);

        for (toki::u32 i = 0; i < vec.get_capacity(); i++) {
            std::print("{} ", vec[i]);
        }
    }

    std::println();

    {
        toki::containers::DynamicArray<char> vec(20, 'A');
        for (toki::u32 i = 0; i < vec.get_capacity(); i++) {
            std::print("{} ", vec[i]);
        }
    }

    while (true) {
        toki::platform::poll_events();
    }

    return 0;
}
