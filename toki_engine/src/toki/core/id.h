
#pragma once

#include <cstdint>
#include <functional>

namespace toki {

class id {
public:
    static id createId();

    id();
    id(uint64_t n);
    id(const id& other);
    id(id&& other);
    id& operator=(const id& other);
    id& operator=(const id&& other);
    id& operator=(const uint64_t&& n);
    ~id() = default;

    uint64_t getId() const;

    b8 operator==(const id& other) const;
    b8 operator<(const id& other) const;
    uint64_t operator()(const id& id) const;

    operator b8() const;

private:
    union {
        uint64_t u64;
        uint32_t u32[2];
    } m_values;
};

}  // namespace toki
