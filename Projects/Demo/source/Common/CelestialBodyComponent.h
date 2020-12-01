#pragma once
#include <EtCore/Util/LinkerUtils.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


namespace et {
namespace demo {


//---------------------------------
// CelestialBodyComponent
//
// Describes the movement of celestial bodies, for now rotation speed
//
struct CelestialBodyComponent final : public fw::SimpleComponentDescriptor
{
	ECS_DECLARE_COMPONENT
	RTTR_ENABLE(fw::SimpleComponentDescriptor) // for serialization
	DECLARE_FORCED_LINKING()
public:

	bool isRotating = false;
	float rotationSpeed = 1.f;
};


} // namespace demo
} // namespace et
