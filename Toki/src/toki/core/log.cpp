#include "tkpch.h"
#include "log.h"
#include "assert.h"
#include "spdlog/spdlog.h"

namespace Toki {

    template<typename ...Args>
    void Log(LogLevel logLevel, std::string tmp, Args&& ...args) {
        switch (logLevel) {
            case LogLevel::Info:
                spdlog::info(tmp, std::forward<Args>(args)...); break;
            case LogLevel::Warn:
                spdlog::warn(tmp, std::forward<Args>(args)...); break;
            case LogLevel::Error:
            case LogLevel::Fatal:
                spdlog::error(tmp, std::forward<Args>(args)...); break;
            case LogLevel::Debug:
            case LogLevel::Trace:
                spdlog::info(tmp, std::forward<Args>(args)...); break;
        }
    }

    void reportAssert(std::string_view expression, std::string_view file, uint32_t line, std::string_view message) {
        if (!message.empty())
            spdlog::error("Assertion error {}:{} - {}\n\t{}", file, line, expression, message);
        else
            spdlog::error("Assertion error {}:{} - {}", file, line, expression);
    }

}