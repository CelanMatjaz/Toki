#pragma once

#include "stdexcept"
#include "format"
#include "string"
#include "cstdint"

namespace Toki {

    void reportAssert(std::string_view expression, std::string_view file, uint32_t line, std::string_view message = "");

}

#ifndef DIST

#define TK_ASSERT(expression, message)                                  \
{                                                                       \
    if(expression);                                                     \
    else {                                                              \
        Toki::reportAssert(#expression, __FILE__, __LINE__, message);         \
        __debugbreak();                                                 \
        exit(1);                                                        \
    }                                                                   \
}                                                                       \

#define TK_ASSERT_VK_RESULT(vkFunctionResult, message)                  \
{                                                                       \
    if(vkFunctionResult == 0);                                          \
    else {                                                              \
        reportAssert(#vkFunctionResult, __FILE__, __LINE__, message);   \
        std::cout << "\tResult value: " << vkFunctionResult << "\n";    \
        __debugbreak();                                                 \
        exit(1);                                                        \
    }                                                                   \
}                                                                       \
// #define TK_ASSERT_VK_RESULT(vkFunctionResult, message, ...) TK_ASSERT(vkFunctionResult == 0 /* VK_SUCCESS value */, message)

#else

#define TK_ASSERT(expression, message)
#define TK_ASSERT_VK_RESULT(vkFunctionResult, message, ...)

#endif