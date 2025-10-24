#pragma once

namespace toki {

enum struct TokiError {
	NoError,
	Unknown,

	FileOpen,

	MEMORY_ALLOCATION_FAILED,
};

}  // namespace toki
