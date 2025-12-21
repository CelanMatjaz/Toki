#include "testing.h"
#include "toki/core/math/matrix4.h"
#include "toki/core/memory/memory.h"
#include "toki/core/utils/bytes.h"
//

#include <toki/core/core.h>

using namespace toki;

TK_TEST(Vector3, add) {
	Vector3 vec1(1, 2, 3);
	Vector3 vec2(2, 3, 4);

	Vector3 vec3 = vec1 + vec2;
	TK_TEST_ASSERT(vec3.x == 3);
	TK_TEST_ASSERT(vec3.y == 5);
	TK_TEST_ASSERT(vec3.z == 7);

	return true;
}

TK_TEST(Vector3, subtract) {
	Vector3 vec1(1, 2, 3);
	Vector3 vec2(2, 3, 4);

	Vector3 vec3 = vec1 - vec2;
	TK_TEST_ASSERT(vec3.x == -1);
	TK_TEST_ASSERT(vec3.y == -1);
	TK_TEST_ASSERT(vec3.z == -1);

	return true;
}

TK_TEST(Vector3, length) {
	Vector3 vec(3, 4, 0);

	TK_TEST_ASSERT(static_cast<u32>(vec.length_squared()) == 25);
	TK_TEST_ASSERT(static_cast<u32>(vec.length()) == 5);

	return true;
}

TK_TEST(Vector3, normalize) {
	Vector3 vec1(1, 0, 0);
	TK_TEST_ASSERT(vec1.normalize() == Vector3(1.0f, 0.0f, 0.0f));

	Vector3 vec2(3, 4, 0);
	TK_TEST_ASSERT(vec2.normalize() == Vector3(0.6f, 0.8f, 0.0f));

	return true;
}

TK_TEST(Vector3, cross) {
	Vector3 vec1(1, 0, 0);
	Vector3 vec2(0, 1, 0);

	Vector3 expected_result1(0, 0, 1);
	Vector3 expected_result2(0, 0, -1);

	TK_TEST_ASSERT(vec1.cross(vec2) == expected_result1);
	TK_TEST_ASSERT(vec2.cross(vec1) == expected_result2);

	return true;
}

TK_TEST(Vector3, fromatter) {
	toki::memory_initialize({ .total_size = toki::MB(10) });

	Vector3 vec(1, 2, 3);

	toki::String string = Formatter<Vector3>::format(vec);
	b8 compare_ok = toki::strncmp(string.data(), "Vector3 [1.000000 2.000000 3.000000]", string.size()) == 0;
	TK_TEST_ASSERT(compare_ok);

	Matrix4 mat{vec};
	toki::String string1 = Formatter<Matrix4>::format(mat);

	return true;
}
