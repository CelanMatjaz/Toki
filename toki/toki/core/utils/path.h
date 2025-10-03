#pragma once

#include <toki/core/string/basic_string.h>
#include <toki/core/string/string_view.h>

namespace toki {

class Path {
public:
	Path();
	Path(const toki::StringView str): m_internalPath(str.to_string()) {}
	~Path() = default;

	const char* c_str() const {
		return m_internalPath.data();
	}

private:
	toki::String m_internalPath;
};

Path current_path();

}  // namespace toki
