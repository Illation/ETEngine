#include "stdafx.h"
#include "registerMath.h"


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

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
}

DEFINE_FORCED_LINKING(MathRegistrationLinkEnforcer) // force the linker to include this unit
