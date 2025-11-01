#pragma once

#include <toki/core/common/optional.h>
#include <toki/core/memory/memory.h>
#include <toki/core/types.h>

namespace toki {

template <typename T, u64 N, typename AllocatorType = DefaultAllocator>
	requires(CIsAllocator<AllocatorType>)
class RingBuffer {
public:
	RingBuffer(): m_data(reinterpret_cast<T*>(AllocatorType::allocate_aligned(N * sizeof(T), alignof(T)))) {}
	~RingBuffer() {
		AllocatorType::free_aligned(m_data);
	}

	bool pop(T& out) {
		if (m_size == 0) {
			return false;
		}

		out	   = m_data[m_head];
		m_head = (m_head + 1) % N;
		m_size--;
		return true;
	}

	bool push_back(T&& value) {
		if (m_size > N) {
			return false;
		}

		m_data[m_tail] = value;
		m_tail		   = (m_tail + 1) % N;
		m_size++;
		return true;
	}

	template <typename... Args>
	bool emplace_back(Args&&... args) {
		if (m_size > N) {
			return false;
		}

		toki::construct_at<T>(&m_data[m_tail], toki::forward<Args>(args)...);
		m_tail = (m_tail + 1) % N;
		m_size++;
		return true;
	}

	bool is_empty() const {
		return m_size == 0;
	}

	bool is_full() const {
		return m_size == N;
	}

	void clear() {
		if constexpr (CHasDestructor<T>) {
			for (u32 i = m_head; i != m_tail; i++) {
				if (i == N) {
					i -= N;
				}

				toki::destroy_at<T>(&m_data[i]);
			}
		}

		m_size = 0;
		m_head = m_tail;
	}

	struct Iterator {
		RingBuffer* ring_buffer;
		i64 index;

		Iterator(RingBuffer* rb, u64 i): ring_buffer(rb), index(i) {}

		T& operator*() {
			return ring_buffer->m_data[index];
		}

		Iterator& operator++() {
			index = (index + 1) % N;
			return *this;
		}

		bool operator!=(const Iterator& other) const {
			return index != other.index;
		}
	};

	Iterator begin() {
		return Iterator(this, m_head);
	}

	Iterator end() {
		return Iterator(this, m_tail);
	}

private:
	T* m_data{};
	u32 m_size{};
	u32 m_head{};
	u32 m_tail{};
};

}  // namespace toki
