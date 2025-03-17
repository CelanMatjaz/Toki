#pragma once

#include "../core/common.h"
#include "../core/concepts.h"
#include "../core/types.h"
#include "../memory/allocators/allocator.h"

namespace toki {

template <typename T, typename A = Allocator>
    requires AllocatorConcept<A>
class BasicRef {
public:
    BasicRef() {};
    BasicRef(A& allocator, u32 element_count = 1):
        mAllocator(&allocator),
        mData(reinterpret_cast<T*>(allocator.allocate(element_count * sizeof(T)))),
        mCapacity(element_count) {}

    ~BasicRef() {
        mAllocator->free(mData);
    }

    BasicRef(const BasicRef& other) = delete;
    BasicRef& operator=(const BasicRef& other) = delete;

    BasicRef(BasicRef<T>&& other) {
        if (this != &other) {
            this->swap(other);
        }
    }

    BasicRef& operator=(BasicRef&& other) {
        if (this != &other) {
            this->swap(remove_r_value_ref(other));
        }

        return *this;
    }

    u64 size() const {
        return mCapacity;
    }

    T* data() const {
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

    operator T*() const {
        return mData;
    }

private:
    inline void swap(BasicRef<T>& other) {
        toki::swap(mAllocator, other.mAllocator);
        toki::swap(mData, other.mData);
        toki::swap(mCapacity, other.mCapacity);
    }

    A* mAllocator{};
    T* mData{};
    u64 mCapacity{};
};

class BumpAllocator;

template <typename T>
using BumpRef = BasicRef<T, BumpAllocator>;

}  // namespace toki
