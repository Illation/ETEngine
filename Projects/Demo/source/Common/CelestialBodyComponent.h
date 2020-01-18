#pragma once
#include <EtCore/Helper/LinkerUtils.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


namespace demo {


//---------------------------------
// CelestialBodyComponent
//
// Describes the movement of celestial bodies, for now rotation speed
//
struct CelestialBodyComponent final
{
	ECS_DECLARE_COMPONENT
public:

	bool isRotating = false;
	float rotationSpeed = 1.f;
};


//---------------------------------
// CelestialBodyComponentDesc
//
class CelestialBodyComponentDesc final : public fw::ComponentDescriptor<CelestialBodyComponent>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<CelestialBodyComponent>)
	DECLARE_FORCED_LINKING()

	// construct destruct
	//--------------------
public:
	CelestialBodyComponentDesc() : ComponentDescriptor<CelestialBodyComponent>() {}

	// ComponentDescriptor interface
	//-------------------------------
	CelestialBodyComponent* MakeData() override;

	float rotationSpeed = 1.f;
	bool startRotating = false;
};


} // namespace demo

