#pragma once

#include "../core/assert.h"
#include "../core/log.h"
#include "../core/types.h"
#include "../memory/memory.h"

namespace toki {

constexpr u32 INVALID_HANDLE_ID = 0;

struct Handle {
	Handle(): index(0), version(0), id(INVALID_HANDLE_ID) {};
	Handle(u32 index, u32 version = 1):
		index(index),
		version(version),
		id(static_cast<u32>(/* Time().as<Time::Unit::Nanoseconds>() */ 0)) {}

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
	u32 data{ 0 };
};

template <typename ValueType>
class HandleMap {
public:
	HandleMap() {}
	HandleMap(u32 element_count, u32 free_list_element_count): m_element_capacity(element_count) {
		u32 free_list_size = free_list_element_count * sizeof(u32);
		u32 version_list_size = element_count * sizeof(u32);
		u32 skip_field_size = element_count * sizeof(u32);
		u32 element_list_size = (static_cast<u64>(element_count) + 1) * sizeof(ValueType);

		u32 total_size = free_list_size + skip_field_size + version_list_size + element_list_size;

		void* ptr = memory_allocate(total_size);
		m_free_list = reinterpret_cast<u32*>(ptr);
		m_version_list = reinterpret_cast<u32*>(m_free_list + free_list_element_count);
		m_skip_field = reinterpret_cast<u32*>(m_version_list + free_list_element_count);
		m_data = reinterpret_cast<ValueType*>(m_skip_field + element_count) + 1;
	}

	~HandleMap() {}

	DELETE_COPY(HandleMap)
	DELETE_MOVE(HandleMap)

	inline void invalidate(Handle& handle) {
		TK_ASSERT(is_handle_valid(handle), "Cannot invalidate an invalid handle");
		++m_version_list[handle.index];
		handle.invalidate();
	}

	inline b8 contains(const Handle handle) const {
		return is_handle_valid(handle);
	}

	inline u32 capacity() const {
		return m_element_capacity;
	}

	template <typename... Args>
	Handle emplace(Args&&... args) {
		i32 free_block_index = get_next_free_block_index();
		new (&m_data[free_block_index]) ValueType(args...);
		u32 version = m_version_list[free_block_index]++;
		return Handle{ static_cast<u32>(free_block_index), version };
	}

	Handle insert(ValueType&& value) {
		i32 free_block_index = get_next_free_block_index();
		memcpy(&value, &m_data[free_block_index], sizeof(ValueType));
		++m_version_list[free_block_index];
		return Handle{ static_cast<u32>(free_block_index), m_version_list[free_block_index] };
	}

	inline ValueType& at(const Handle& handle) const {
		TK_ASSERT(is_handle_valid(handle), "Invalid handle provided");
		return m_data[handle.index];
	}

	inline ValueType& operator[](const Handle& handle) const {
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
	inline b8 is_handle_valid(const Handle& handle) const {
		return handle.valid() && handle.version == is_version_valid(handle);
	}

	inline b8 is_version_valid(const Handle& handle) const {
		TK_ASSERT(handle.index <= m_element_capacity, "Handle index invalid");
		return m_version_list[handle.index] == handle.version;
	}

	inline b8 is_handle_in_range(const Handle handle) const {
		return handle.index < m_element_capacity;
	}

	i32 get_next_free_block_index() {
		if (m_free_list_size <= 0 && m_next_free_block_index >= m_element_capacity) {
			toki ::print_args(
				toki ::LOG_LEVEL_STRINGS[static_cast<toki ::u32>(toki ::LogLevel ::TRACE)],
				"HandleMap's capacity reached, returning index to NOOP object");
			return -1;
		}

		TK_ASSERT(m_free_list_size > 0 || m_next_free_block_index < m_element_capacity, "Handle map is full");

		if (m_free_list_size > 0) {
			return m_free_list[(m_free_list_size--) - 1];
		}

		return m_next_free_block_index++;
	}

	u32* m_free_list{};
	u32* m_version_list{};
	u32* m_skip_field{};
	ValueType* m_data{};
	u32 m_free_list_size{};
	u32 m_element_capacity{};
	u32 m_next_free_block_index{};

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
