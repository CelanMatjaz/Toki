#pragma once

#include <cstdint>

namespace Toki {

class Id {
public:
    static Id createId();

    Id();
    Id(const Id& other);
    Id(Id&& other);
    Id& operator=(const Id& other);
    Id& operator=(const Id&& other);
    ~Id() = default;

    bool operator==(const Id& other) const;
    size_t operator()(const Id& p) const;

private:
    uint32_t m_values[2];
};

}  // namespace Toki
