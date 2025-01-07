
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

    bool operator==(const id& other) const;
    bool operator<(const id& other) const;
    uint64_t operator()(const id& id) const;

    operator bool() const;

private:
    union {
        uint64_t u64;
        uint32_t u32[2];
    } m_values;
};

}  // namespace toki
