#pragma once

#include <expected>
#include <optional>

#ifndef TK_NDEBUG
#include <stacktrace>
#endif

#include "toki/core/logging.h"

namespace Toki {

#define CREATE_ERRORS(...)        \
    enum class Error : uint32_t { \
        __VA_ARGS__               \
    };                            \
                                  \
    class ErrorHandler {          \
        inline static const char* s_errorStrings[] = { #__VA_ARGS__ };

// Create errors and make error strings private
CREATE_ERRORS(
    NoError = 0,
    // I/O
    FileNotFoundError,
    FileReadError,
    FileOpenError,
    FileParseError,

    // Resource errors
    ResourcePathNotFound,
    ResourceNotLoaded)

public:
template <typename T>
static std::optional<T> checkForError(const std::expected<T, Error>&);
};

template <typename T>
std::optional<T> ErrorHandler::checkForError(const std::expected<T, Error>& ref) {
    if (!ref.has_value() && ref.error() != Error::NoError) {
#ifndef TK_NDEBUG
        LOG_ERROR("{}: Stacktrace {}\n", s_errorStrings[(uint32_t) ref.error()], std::stacktrace::current());
#else
        LOG_ERROR("{}", s_errorStrings[(uint32_t) ref.error()]);
#endif
        return {};
    }

    return ref.value();
}

#define CHECK_ERROR(check) ErrorHandler::checkForError(check)

}  // namespace Toki
