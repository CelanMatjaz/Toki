#include "id.h"

#include <cstring>
#include <ctime>

namespace Toki {

Id Id::createId() {
    return Id();
}

static uint32_t id = 0;

Id::Id() : m_values{ id++, (uint32_t) time(0) } {}

Id::Id(const Id& other) {
    memcpy(m_values, other.m_values, sizeof(m_values));
}

Id::Id(Id&& other) {
    memcpy(m_values, other.m_values, sizeof(m_values));
}

Id& Id::operator=(const Id& other) {
    if (this == &other) {
        return *this;
    }

    memcpy(m_values, other.m_values, sizeof(m_values));
    return *this;
}

Id& Id::operator=(const Id&& other) {
    if (this == &other) {
        return *this;
    }

    memcpy(m_values, other.m_values, sizeof(m_values));
    return *this;
}

bool Id::operator==(const Id& other) const {
    return m_values[0] == other.m_values[0] && m_values[1] == other.m_values[1];
}

size_t Id::operator()(const Id& p) const {
    return (size_t) (m_values[0] << sizeof(m_values[0]) | m_values[1]);
}

}  // namespace Toki
