#pragma once

#include <filesystem>

namespace Toki {

namespace Core {

enum LogFlags {
    LOG_CONSOLE = 1 << 0,
    LOG_FILE = 1 << 1,
};

void logging_initialize(LogFlags flags, std::filesystem::path path = "");
void logging_shutdown();

void open_log_file_stream(std::filesystem::path path);
void close_current_log_file_stream();

void log(const char* tag, std::string log_string);
void log_error(const char* tag, std::string log_string);
void log_file(std::string print_string);

}  // namespace Core

}  // namespace Toki

#ifdef TK_DIST

#else

#define TK_LOG_INFO(str, ...) Toki::Core::log("Info", std::format(str __VA_OPT__(, ) __VA_ARGS__))
#define TK_LOG_WARN(str, ...) Toki::Core::log("Warn", std::format(str __VA_OPT__(, ) __VA_ARGS__))
#define TK_LOG_ERROR(str, ...) Toki::Core::log_error("Error", std::format(str __VA_OPT__(, ) __VA_ARGS__))
#define TK_LOG_FATAL(str, ...) Toki::Core::log_error("Fatal", std::format(str __VA_OPT__(, ) __VA_ARGS__))
// #define TK_LOG_TRACE(str, ...) Toki::Core::log_error("Trace", std::format(str __VA_OPT__(, ) __VA_ARGS__))

#endif
