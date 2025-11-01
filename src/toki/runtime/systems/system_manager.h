#pragma once

#include <toki/runtime/systems/font_system.h>

namespace toki {

struct SystemManagerConfig {};

class SystemManager {
public:
	static toki::UniquePtr<SystemManager> create(const SystemManagerConfig& config);

	SystemManager() = delete;
	SystemManager(const SystemManagerConfig& config);

	FontSystem& font_system();

private:
	FontSystem m_fontSystem;
};

}  // namespace toki
