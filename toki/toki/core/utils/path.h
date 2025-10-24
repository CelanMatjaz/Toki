#pragma once

#include <toki/core/string/basic_string.h>
#include <toki/core/string/string_view.h>

namespace toki {

class Path {
public:
	Path() = default;
	Path(const char* str): m_internalPath(str) {}
	Path(char* str): m_internalPath(str) {}
	Path(toki::StringView str): m_internalPath(str.to_string()) {}
	~Path() = default;

	Path& operator=(toki::StringView str) {
		m_internalPath = str.to_string();
		return *this;
	}

	const char* c_str() const {
		return m_internalPath.data();
	}

private:
	toki::String m_internalPath;
};

}  // namespace toki
