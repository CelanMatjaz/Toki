#pragma once

#include "platform/platform.h"
#include "print.h"

namespace toki {

enum struct LogLevel {
	Info,
	Warn,
	Error,
	Fatal,
	Debug,
	Trace
};

consteval const char* const level_to_string(LogLevel level) {
	switch (level) {
		case LogLevel::Info:
			return "INFO";
		case LogLevel::Warn:
			return "WARN";
		case LogLevel::Error:
			return "ERROR";
		case LogLevel::Fatal:
			return "FATAL";
		case LogLevel::Debug:
			return "DEBUG";
		case LogLevel::Trace:
			return "TRACE";
	}

	TK_UNREACHABLE();
}

#define TK_LOG(str) toki::println(str);
#define TK_LOG_INFO(str) toki::println(str);
#define TK_LOG_WARN(str) toki::println(str);
#define TK_LOG_FATAL(str) toki::println(str);
#define TK_LOG_DEBUG(str) toki::println(str);
#define TK_LOG_TRACE(str) toki::println(str);

}  // namespace toki
