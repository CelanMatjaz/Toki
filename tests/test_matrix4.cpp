#include "testing.h"
#include "toki/core/common/assert.h"
#include "toki/core/common/print.h"
//

#include <toki/core/core.h>

using namespace toki;

TK_TEST(Matrix4, multiply) {
	Matrix4 identity{};
	Matrix4 T{ Vector3{ 1, 2, 3 } };

	Matrix4 S{};
	S.m[0] = 2.0f;
	S.m[5] = 2.0f;
	S.m[10] = 2.0f;
	S.m[15] = 1.0f;

	Matrix4 M = T * S;

	Matrix4 expected(
		// column 0
		2.0f,
		0.0f,
		0.0f,
		0.0f,
		// column 1
		0.0f,
		2.0f,
		0.0f,
		0.0f,
		// column 2
		0.0f,
		0.0f,
		2.0f,
		0.0f,
		// column 3
		1.0f,
		2.0f,
		3.0f,
		1.0f);

	TK_TEST_ASSERT(M == expected);

	return true;
}
