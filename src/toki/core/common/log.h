#pragma once

#include <toki/core/string/string_view.h>
#include <toki/core/types.h>

namespace toki {

template <typename... Args>
void println(const StringView str, Args&&... args);

#define LOG_LEVELS(X)      \
	X(INFO, "\033[0;37m")  \
	X(WARN, "\033[0;36m")  \
	X(ERROR, "\033[0;33m") \
	X(FATAL, "\033[0;31m") \
	X(DEBUG, "\033[0;35m") \
	X(TRACE, "\033[0;36m")

enum struct LogLevel : u32 {
#define X(name, color) name,
	LOG_LEVELS(X)
#undef X
};

inline const char* LOG_LEVEL_STRINGS[] = {
#define X(name, color) color "[" #name "]",
	LOG_LEVELS(X)
#undef X
};

#define _LOG_PRINT(level, str, ...) \
	toki::println(                  \
		"{} " str "\033[0m",        \
		toki::LOG_LEVEL_STRINGS[static_cast<toki::u32>(toki::LogLevel::level)] __VA_OPT__(, ) __VA_ARGS__)

#if defined(LOG_INFO_OFF) || defined(LOGGING_OFF)
	#define TK_LOG_INFO(message, ...)
#else
	#define TK_LOG_INFO(...) _LOG_PRINT(INFO __VA_OPT__(, ) __VA_ARGS__)
#endif

#if defined(LOG_WARN_OFF) || defined(LOGGING_OFF)
	#define TK_LOG_WARN(message, ...)
#else
	#define TK_LOG_WARN(...) _LOG_PRINT(WARN __VA_OPT__(, ) __VA_ARGS__)
#endif

#if defined(LOG_ERROR_OFF) || defined(LOGGING_OFF)
	#define TK_LOG_ERROR(message, ...)
#else
	#define TK_LOG_ERROR(...) _LOG_PRINT(ERROR __VA_OPT__(, ) __VA_ARGS__)
#endif

#if defined(LOG_FATAL_OFF) || defined(LOGGING_OFF)
	#define TK_LOG_FATAL(message, ...)
#else
	#define TK_LOG_FATAL(...) _LOG_PRINT(FATAL __VA_OPT__(, ), __VA_ARGS__)
#endif

#if defined(LOG_DEBUG_OFF) || defined(LOGGING_OFF)
	#define TK_LOG_DEBUG(message, ...)
#else
	#define TK_LOG_DEBUG(...) _LOG_PRINT(DEBUG __VA_OPT__(, ) __VA_ARGS__)
#endif

#if defined(LOG_TRACE_OFF) || defined(LOGGING_OFF)
	#define TK_LOG_TRACE(message, ...)
#else
	#define TK_LOG_TRACE(...) _LOG_PRINT(TRACE __VA_OPT__(, ) __VA_ARGS__)
#endif

}  // namespace toki
