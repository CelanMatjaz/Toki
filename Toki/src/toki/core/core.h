#pragma once

#define ENGINE_NAME "Toki"
#define ENGINE_VERSION 1

#define TK_ASSERT(assertCondition) if(assertCondition); else throw std::runtime_error(std::format("Assertion error {}:{} - {}\n", __FILE__, __LINE__, #assertCondition))
#define TK_ASSERT_VK_RESULT(vkFunction) if(vkFunction == VK_SUCCESS); else throw std::runtime_error(std::format("Assertion error {}-{}:{}", #vkFunction, __FILE__, __LINE__))

namespace Toki {

    template<typename T>
    using TkScope = std::unique_ptr<T>;
    template<typename T, typename ...Args>
    constexpr TkScope<T> createScope(Args&& ...args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using TkRef = std::shared_ptr<T>;
    template<typename T, typename ...Args>
    constexpr TkRef<T> createRef(Args&& ...args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

}