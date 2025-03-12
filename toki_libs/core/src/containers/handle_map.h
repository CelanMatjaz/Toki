#pragma once

#include "../core/assert.h"
#include "../core/common.h"
#include "../core/logging.h"
#include "../core/types.h"
#include "../platform/platform.h"

namespace toki {

constexpr u32 INVALID_HANDLE_ID = 0;

struct Handle {
    Handle() = delete;
    Handle(u32 index, u32 version = 1): index(index), version(version), id(platform::get_time_milliseconds()) {}

    inline operator b8() const {
        return id != INVALID_HANDLE_ID;
    }

    inline b8 valid() const {
        return this->operator bool();
    }

    inline void invalidate() {
        id = INVALID_HANDLE_ID;
    }

    u32 index{ 0 };
    u32 version{ 0 };
    u32 id{ INVALID_HANDLE_ID };
};

template <typename ValueType>
class HandleMap {
public:
    HandleMap() {}
    HandleMap(AllocatorConcept auto& allocator, u32 element_count, u32 free_list_element_count):
        mElementCapacity(element_count) {
        u32 free_list_size = free_list_element_count * sizeof(u32);
        u32 version_list_size = element_count * sizeof(u32);
        u32 skip_field_size = element_count * sizeof(u32);
        u32 element_list_size = (element_count + 1) * sizeof(ValueType);

        u32 total_size = free_list_size + skip_field_size + version_list_size + element_list_size;

        void* ptr = allocator.allocate(total_size);
        mFreeList = reinterpret_cast<u32*>(ptr);
        mVersionList = reinterpret_cast<u32*>(mFreeList + free_list_element_count);
        mSkipField = reinterpret_cast<u32*>(mVersionList + free_list_element_count);
        mData = reinterpret_cast<ValueType*>(mSkipField + element_count) + 1;
    }

    ~HandleMap() {}

    u32* mFreeList{};
    u32* mVersionList{};
    u32* mSkipField{};
    ValueType* mData{};
    u32 mFreeListSize{};
    u32 mElementCapacity{};
    u32 mNextFreeBlockIndex{};

    DELETE_COPY(HandleMap)
    DELETE_MOVE(HandleMap)

    inline void invalidate(Handle& handle) {
        TK_ASSERT(is_handle_valid(handle), "Cannot invalidate an invalid handle");
        ++mVersionList[handle.index];
        handle.invalidate();
    }

    inline b8 is_handle_valid(const Handle& handle) const {
        return handle.valid() && handle.version == is_version_valid(handle);
    }

    inline b8 is_version_valid(const Handle& handle) const {
        TK_ASSERT(handle.index <= mElementCapacity, "Handle index invalid");
        return mVersionList[handle.index] == handle.version;
    }

    inline b8 contains(const Handle handle) const {
        return is_handle_valid(handle);
    }

    inline u32 capacity() const {
        return mElementCapacity;
    }

    template <typename... Args>
    Handle emplace(Args&&... args) {
        i32 free_block_index = get_next_free_block_index();
        new (&mData[free_block_index]) ValueType(args...);
        u32 version = mVersionList[free_block_index]++;
        return Handle{ static_cast<u32>(free_block_index), version };
    }

    Handle insert(ValueType&& value) {
        i32 free_block_index = get_next_free_block_index();
        memcpy(&value, &mData[free_block_index], sizeof(ValueType));
    }

    inline ValueType& at(const Handle handle) const {
        TK_ASSERT(is_handle_valid(handle), "Invalid handle provided");
        return mData[handle.index];
    }

    inline ValueType& operator[](const Handle handle) const {
        return at(handle);
    }

    /* class Iterator;

    Iterator begin() {
        return Iterator(&mData, 0);
    }

    Iterator end() {
        return Iterator(&mData, mData.element_capacity);
    } */

private:
    b8 is_handle_in_range(const Handle handle) const {
        return handle.index < mElementCapacity;
    }

    i32 get_next_free_block_index() {
        if (mFreeListSize <= 0 && mNextFreeBlockIndex >= mElementCapacity) {
            TK_LOG_TRACE("HandleMap's capacity reached, returning index to NOOP object");
            return -1;
        }

        TK_ASSERT(mFreeListSize > 0 || mNextFreeBlockIndex < mElementCapacity, "Handle map is full");

        if (mFreeListSize > 0) {
            return mFreeList[(mFreeListSize--) - 1];
        }

        return mNextFreeBlockIndex++;
    }

public:
    /* class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = ValueType;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        Iterator() = delete;
        Iterator(InternalData* data, u64 index): m_data(data), m_index(index) {}
        Iterator(const Iterator& other) = default;
        Iterator& operator=(const Iterator& other) = default;

        reference operator*() const {
            return (m_data->values_ptr)[m_index];
        }

        pointer operator->() const {
            return &(m_data->values_ptr)[m_index];
        }

        Iterator& operator++() {
            ++m_index;
            auto v = m_data->ptr[m_index];

            if (auto ptr = m_data->ptr[m_index]; ptr == nullptr) {
                while (m_data->ptr[++m_index] != nullptr && m_index <= m_data->last_allocated_index)
                    ;
            }

            if (m_index > m_data->last_allocated_index) {
                m_index = m_data->element_capacity;
            }

            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            ++m_index;
            return temp;
        }

        bool operator==(const Iterator& other) const {
            return m_index == other.m_index;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }

    private:
        InternalData* m_data{};
        u64 m_index{};
    }; */
};

}  // namespace toki
