#include <print>

#include "platform/platform_io.h"
#include "toki.h"

int main() {
    using namespace toki::platform;
    file_delete("test.txt");
    std::println("exists {}", file_exists("test.txt"));
    File _ = file_open("test.txt", (FileOpen_CreateNew | FileOpen_ReadWrite));
    std::println("exists {}", file_exists("test.txt"));

    directory_create("test_dir/test_recursive");
}
