#pragma once

#include <toki/core/types.h>

namespace toki {

#define LOG_LEVELS(X) \
	X(INFO)           \
	X(WARN)           \
	X(ERROR)          \
	X(FATAL)          \
	X(DEBUG)          \
	X(TRACE)

enum struct LogLevel : u32 {
#define X(name) name,
	LOG_LEVELS(X)
#undef X
};

inline const char* LOG_LEVEL_STRINGS[] = {
#define X(name) "[" #name "]",
	LOG_LEVELS(X)
#undef X
};

#define _LOG_PRINT(level, str, ...) \
	toki::println(                  \
		"{} " str, toki::LOG_LEVEL_STRINGS[static_cast<toki::u32>(toki::LogLevel::level)] __VA_OPT__(, ) __VA_ARGS__)

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
