#include "stdafx.h"
#include "registerMath.h"


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<etm::vector<2, int32>>("ivec2")
		.property("x", &ivec2::x)
		.property("y", &ivec2::y);

	registration::class_<etm::vector<2, float>>("vec2")
		.property("x", &vec2::x)
		.property("y", &vec2::y);

	registration::class_<etm::vector<3, float>>("vec3")
		.property("x", &vec3::x)
		.property("y", &vec3::y)
		.property("z", &vec3::z);

	registration::class_<etm::vector<4, float>>("vec4")
		.property("x", &vec4::x)
		.property("y", &vec4::y)
		.property("z", &vec4::z)
		.property("w", &vec4::w);

	registration::class_<etm::quaternion<float>>("quat")
		.property("x", &quat::x)
		.property("y", &quat::y)
		.property("z", &quat::z)
		.property("w", &quat::w);

	registration::class_<etm::matrix<3, 3, float>>("mat3")
		.property("rows", &mat3::rows);

	registration::class_<etm::matrix<4, 4, float>>("mat4")
		.property("rows", &mat4::rows);
}

DEFINE_FORCED_LINKING(MathRegistrationLinkEnforcer) // force the linker to include this unit
