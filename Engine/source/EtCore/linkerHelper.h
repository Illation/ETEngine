#pragma once
#include <EtCore/Content/AssetStub.h>
#include <EtCore/Reflection/registerMath.h>
#include <EtCore/Hashing/HashString.h>


namespace et {
namespace core {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	FORCE_LINKING(MathRegistrationLinkEnforcer)
	FORCE_LINKING(HashString)
	FORCE_LINKING(StubAsset)
}


} // namespace core
} // namespace et

