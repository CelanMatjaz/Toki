
#pragma once

#include <cstdint>
#include <functional>

namespace toki {

class Id {
public:
    static Id createId();

    Id();
    Id(uint64_t n);
    Id(const Id& other);
    Id(Id&& other);
    Id& operator=(const Id& other);
    Id& operator=(const Id&& other);
    Id& operator=(const uint64_t&& n);
    ~Id() = default;

    uint64_t getId() const;

    bool operator==(const Id& other) const;
    bool operator<(const Id& other) const;
    uint64_t operator()(const Id& id) const;

    operator bool() const;

private:
    union {
        uint64_t u64;
        uint32_t u32[2];
    } m_values;
};

using Handle = Id;

}  // namespace Toki
