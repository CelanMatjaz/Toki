#pragma once

#include "../core/common.h"
#include "../core/concepts.h"
#include "../core/types.h"
#include "../memory/allocator.h"

namespace toki {

template <typename T, typename A = Allocator>
    requires AllocatorConcept<A>
class BasicRef {
public:
    BasicRef() {};
    BasicRef(A& allocator, u32 element_count = 1, const T* data = nullptr):
        mAllocator(&allocator),
        mData(reinterpret_cast<T*>(allocator.allocate(element_count * sizeof(T) + sizeof(u64)))),
        mCapacity(element_count) {
        if (data != nullptr) {
            toki::memcpy(data, mData, element_count * sizeof(T));
        }
    }

    ~BasicRef() {
        --(*reinterpret_cast<u64*>(mData));
        if (*reinterpret_cast<u64*>(mData)) {
            mAllocator->free(mData);
        }
    }

    BasicRef(const BasicRef& other): mAllocator(other.mAllocator), mData(other.mData), mCapacity(other.mCapacity) {
        ++(*reinterpret_cast<u64*>(mData));
    }

    BasicRef& operator=(const BasicRef& other) {
        mAllocator = other.mAllocator;
        mData = other.mData;
        mCapacity = other.mCapacity;
        ++(*reinterpret_cast<u64*>(mData));
    }

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

    inline u64 size() const {
        return mCapacity;
    }

    inline T* data() const {
        return reinterpret_cast<T*>(reinterpret_cast<u64*>(mData) + 1);
    }

    inline operator bool() const {
        return mData != nullptr;
    }

    inline bool valid() const {
        return mData != nullptr;
    }

    T& operator[](u32 index) const {
        TK_ASSERT(index < mCapacity, "Index out of bounds");
        return data()[index];
    }

    inline T* operator->() const {
        return mData;
    }

    inline operator T*() const {
        return mData;
    }

    inline operator T&() const {
        return *mData;
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
