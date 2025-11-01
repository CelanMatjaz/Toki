#pragma once

namespace toki {

template <typename T, typename Friend = void>
class ConstWrapper {
	friend Friend;

public:
	ConstWrapper() = default;
	ConstWrapper(const T& obj): m_object(&const_cast<T&>(obj)) {}

	const T* operator->() {
		return &get();
	}

	operator const T*() {
		return &get();
	}

	operator const T&() {
		TK_ASSERT(m_object != nullptr);
		return &m_object;
	}

	const T& get() {
		TK_ASSERT(m_object != nullptr);
		return *m_object;
	}

private:
	T* m_object{};
};

}  // namespace toki
