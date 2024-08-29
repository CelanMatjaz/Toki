#pragma once

#include <filesystem>

namespace Toki {

enum LogFlags {
    TK_LOG_FLAGS_CONSOLE = 1 << 0,
    TK_LOG_FLAGS_FILE = 1 << 1,
};

void engine_logging_initialize(uint32_t log_flags, std::filesystem::path path = "");
void engine_logging_shutdown();

void engine_open_log_file_stream(std::filesystem::path path);
void engine_close_current_log_file_stream();

void engine_log(const char* tag, const std::string& log_string);
void engine_log_error(const char* tag, const std::string& log_string);
void engine_log_file(std::string print_string);

}  // namespace Toki

#ifdef TK_DIST

#else

#define TK_LOG_INFO(str, ...) Toki::Core::log("Info", std::format(str __VA_OPT__(, ) __VA_ARGS__))
#define TK_LOG_WARN(str, ...) Toki::Core::log("Warn", std::format(str __VA_OPT__(, ) __VA_ARGS__))
#define TK_LOG_ERROR(str, ...) Toki::Core::log_error("Error", std::format(str __VA_OPT__(, ) __VA_ARGS__))
#define TK_LOG_FATAL(str, ...) Toki::Core::log_error("Fatal", std::format(str __VA_OPT__(, ) __VA_ARGS__))
// #define TK_LOG_TRACE(str, ...) Toki::Core::log_error("Trace", std::format(str __VA_OPT__(, ) __VA_ARGS__))

#endif
