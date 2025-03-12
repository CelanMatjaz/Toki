#pragma once

#include "../core/concepts.h"
#include "../core/macros.h"
#include "../core/types.h"

namespace toki {

template <typename T>
class BasicRef {
public:
    BasicRef() = delete;
    BasicRef(AllocatorConcept auto& allocator, u32 element_count = 1):
        mData(allocator.allocate(element_count * sizeof(T))),
        mCapacity(element_count) {}

    DELETE_COPY(BasicRef)
    DELETE_MOVE(BasicRef)

    u64 count() const {
        return mCapacity;
    }

    T* get() const {
        return mData;
    }

    operator bool() const {
        return mData != nullptr;
    }

    T& operator[](u32 index) const {
        TK_ASSERT(index < mCapacity, "Out of bounds index");
        return mData[index];
    }

    T* operator->() const {
        return mData;
    }

private:
    T* mData{};
    u64 mCapacity{};
};

}  // namespace toki
