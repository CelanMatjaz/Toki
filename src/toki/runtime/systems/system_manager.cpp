#include <toki/runtime/systems/system_manager.h>

namespace toki {

toki::UniquePtr<SystemManager> SystemManager::create(const SystemManagerConfig& config) {
	return toki::make_unique<SystemManager>(config);
}

SystemManager::SystemManager([[maybe_unused]] const SystemManagerConfig& config) {}

FontSystem& SystemManager::font_system() {
	return m_fontSystem;
}

}  // namespace toki
