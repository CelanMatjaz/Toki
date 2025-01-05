#include "id.h"

#include <ctime>

namespace toki {

Id Id::createId() {
    return Id();
}

static uint32_t id = 0;

Id::Id(): m_values{ .u32{ ~(id++), (uint32_t) time(0) } } {}

Id::Id(uint64_t n): m_values{ n } {}

Id::Id(const Id& other) {
    m_values.u64 = other.m_values.u64;
}

Id::Id(Id&& other) {
    m_values.u64 = other.m_values.u64;
}

uint64_t Id::getId() const {
    return m_values.u64;
}

Id& Id::operator=(const Id& other) {
    if (this == &other) {
        return *this;
    }

    m_values.u64 = other.m_values.u64;
    return *this;
}

Id& Id::operator=(const Id&& other) {
    if (this == &other) {
        return *this;
    }

    m_values.u64 = other.m_values.u64;
    return *this;
}

Id& Id::operator=(const uint64_t&& n) {
    m_values.u64 = n;
    return *this;
}

bool Id::operator==(const Id& other) const {
    return m_values.u64 == other.m_values.u64;
}

bool Id::operator<(const Id& other) const {
    return m_values.u64 < other.m_values.u64;
}

uint64_t Id::operator()(const Id& id) const {
    return std::hash<int>()(id.m_values.u32[0]) ^ (std::hash<int>()(id.m_values.u32[1]) << 1);
}

Id::operator bool() const {
    return m_values.u64 != 0;
}

}  // namespace toki
