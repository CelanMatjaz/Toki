#include <toki/core/platform/memory.h>
#include <toki/core/utils/bytes.h>
#include <toki/core/utils/format.h>
#include <toki/core/utils/print.h>

#include <print>

int main() {
	{
		toki::MemoryConfig memory_config{};
		memory_config.total_size = toki::GB(1);
		toki::memory_initialize(memory_config);
	}

	char adwdaw[] = "Dawjio";
	// auto string = toki::format(
	// 	toki::StringView{ "djwao {} ijio {} tee hee\n{}\n{}" }, "koala", 123, true, static_cast<void*>(adwdaw));
	// std::println("{}", string.data());
	// std::println("{}", static_cast<void*>(adwdaw));

	toki::print("djwao {} ijio {{}} tee hee\n{{}}\n{{}}", "koala");
}
