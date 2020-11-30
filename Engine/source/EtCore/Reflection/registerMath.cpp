#include "stdafx.h"
#include "registerMath.h"


namespace et {
namespace core {


HashString const MathMeta::s_VectorType = HashString("VectorType");

// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<math::vector<2, int32>>("ivec2") (metadata(MathMeta::s_VectorType, true))
		.property("data", &ivec2::data);

	registration::class_<math::vector<2, float>>("vec2") (metadata(MathMeta::s_VectorType, true))
		.property("data", &vec2::data);

	registration::class_<math::vector<3, float>>("vec3") (metadata(MathMeta::s_VectorType, true))
		.property("data", &vec3::data);

	registration::class_<math::vector<4, float>>("vec4") (metadata(MathMeta::s_VectorType, true))
		.property("data", &vec4::data);

	registration::class_<math::quaternion<float>>("quat") (metadata(MathMeta::s_VectorType, true))
		.property("data", &quat::data);

	registration::class_<math::matrix<3, 3, float>>("mat3") (metadata(MathMeta::s_VectorType, true))
		.property("data", &mat3::data);

	registration::class_<math::matrix<4, 4, float>>("mat4") (metadata(MathMeta::s_VectorType, true))
		.property("data", &mat4::data);
}

DEFINE_FORCED_LINKING(MathRegistrationLinkEnforcer) // force the linker to include this unit


} // namespace core
} // namespace et
