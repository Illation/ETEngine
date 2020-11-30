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


} // namespace core
} // namespace et
