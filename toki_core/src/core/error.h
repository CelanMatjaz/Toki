#pragma once

#include "core.h"

namespace toki {

#define ERRORS(X) X(RendererInit)

#define ENUM_ERROR(NAME) NAME,
#define STRING_ERROR(NAME) #NAME,

enum Error : u64 {
    NoError = 0,

    ERRORS(ENUM_ERROR) ERROR_COUNT
};

inline const byte* error_to_string(enum Error value) {
    static const byte* error_strings[] = { ERRORS(STRING_ERROR) };
    return error_strings[static_cast<u64>(value)];
}

#undef ERRORS
#undef ENUM_ERROR
#undef STRING_ERROR

struct TkError {
    TkError(): error(Error::NoError) {}
    TkError(enum Error error): error(error) {}

    operator enum Error() const {
        return error;
    }

    operator bool() const {
        return error == Error::NoError;
    }

    const byte* to_string() const {
        return error_to_string(error);
    }

public:
    enum Error error;
};

}  // namespace toki
