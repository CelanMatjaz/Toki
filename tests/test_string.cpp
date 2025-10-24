#include "testing.h"
//

#include <toki/core/core.h>

using namespace toki;

TK_TEST(BasicString, should_allocate_correctly) {
	toki::String string = "abcdefghijklmnopqrstuvwxyz";
	
	return true;
}
