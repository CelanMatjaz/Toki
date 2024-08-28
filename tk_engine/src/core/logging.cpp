#include "logging.h"

#include <toki/core.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <print>

namespace Toki {

namespace Core {

#ifdef TK_DIST

#else

struct LoggingState {
    const std::chrono::time_zone* current_zone = std::chrono::get_tzdb().current_zone();
    std::ofstream LOG_FILE_STREAM;
    std::mutex print_lock;
    std::mutex file_write_lock;
    std::filesystem::path log_file_path;
    uint32_t flags;
} static s_logging_state;

void logging_initialize(LogFlags flags, std::filesystem::path path) {
    TK_ASSERT(s_logging_state.flags == 0, "Logging already initialized");
    TK_ASSERT(flags != 0, "Cannot initialize logging without providing LogFlags");
    s_logging_state.flags = flags;
    if (s_logging_state.flags & LOG_CONSOLE) {}
    if (s_logging_state.flags & LOG_FILE) {
        std::scoped_lock lock(s_logging_state.file_write_lock);
        TK_ASSERT(!path.empty(), "Path for log file cannot be empty when TK_LOG_FILE flag is provided");
        s_logging_state.LOG_FILE_STREAM.open(path, std::ios::app | std::ios::out);
        TK_ASSERT(s_logging_state.LOG_FILE_STREAM.good(), "File {} not opened for writing", path.string());
    }
}

void logging_shutdown() {
    {
        std::scoped_lock lock(s_logging_state.file_write_lock);
        s_logging_state.LOG_FILE_STREAM.close();
    }

    s_logging_state.flags = 0;
}

void open_log_file_stream(std::filesystem::path path) {
    std::scoped_lock lock(s_logging_state.file_write_lock);
    s_logging_state.LOG_FILE_STREAM = std::ofstream(s_logging_state.log_file_path = path, std::ios::app | std::ios::out);
}

void close_current_log_file_stream() {
    std::scoped_lock lock(s_logging_state.file_write_lock);
    s_logging_state.LOG_FILE_STREAM.close();
}

auto get_time() {
    return std::chrono::zoned_time{ s_logging_state.current_zone,
                                    std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()) };
}

void _log(std::ostream* stream, const char* tag, const std::string& log_string) {
    TK_ASSERT(s_logging_state.flags != 0, "Logging state not initialized");
    auto time = get_time();
    auto print_string = std::format("[{:%T}] {}: {}\n", time, tag, log_string);

    if (s_logging_state.flags & LOG_CONSOLE) {
        *stream << print_string;
    }

    if (s_logging_state.flags & LOG_FILE) {
        std::scoped_lock lock(s_logging_state.print_lock);
        TK_ASSERT(s_logging_state.LOG_FILE_STREAM.good(), "Error writing to log file {}", s_logging_state.log_file_path.string());
        s_logging_state.LOG_FILE_STREAM << print_string;
    }
}

void log(const char* tag, std::string log_string) {
    _log(&std::cout, tag, log_string);
}

void log_error(const char* tag, std::string log_string) {
    _log(&std::cerr, tag, log_string);
}

#undef _LOG

#endif

}  // namespace Core

}  // namespace Toki
