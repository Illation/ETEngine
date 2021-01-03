#pragma once
#include <EtCore/Util/LinkerUtils.h>


namespace et {
namespace core {


class MathMeta
{
public:
	static HashString const s_VectorType;
};

class MathRegistrationLinkEnforcer
{
	DECLARE_FORCED_LINKING()
};

bool IsVectorType(rttr::type const type);


} // namespace core
} // namespace et
