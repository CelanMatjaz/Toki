#include "id.h"

#include <ctime>

namespace toki {

id id::createId() {
    return id();
}

static uint32_t static_id = 0;

id::id(): m_values{ .u32{ ~(static_id++), (uint32_t) time(0) } } {}

id::id(uint64_t n): m_values{ n } {}

id::id(const id& other) {
    m_values.u64 = other.m_values.u64;
}

id::id(id&& other) {
    m_values.u64 = other.m_values.u64;
}

uint64_t id::getId() const {
    return m_values.u64;
}

id& id::operator=(const id& other) {
    if (this == &other) {
        return *this;
    }

    m_values.u64 = other.m_values.u64;
    return *this;
}

id& id::operator=(const id&& other) {
    if (this == &other) {
        return *this;
    }

    m_values.u64 = other.m_values.u64;
    return *this;
}

id& id::operator=(const uint64_t&& n) {
    m_values.u64 = n;
    return *this;
}

b8 id::operator==(const id& other) const {
    return m_values.u64 == other.m_values.u64;
}

b8 id::operator<(const id& other) const {
    return m_values.u64 < other.m_values.u64;
}

uint64_t id::operator()(const id& id) const {
    return std::hash<int>()(id.m_values.u32[0]) ^ (std::hash<int>()(id.m_values.u32[1]) << 1);
}

id::operator b8() const {
    return m_values.u64 != 0;
}

}  // namespace toki
