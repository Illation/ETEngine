#pragma once
#include <EtCore/Helper/LinkerUtils.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


// fwd
namespace fw {
	class EcsController;
}


namespace demo {


//---------------------------------
// SwirlyLightComponent
//
// Component describing the movement state of a light that moves around a point in "swirly" motions
//
struct SwirlyLightComponent final
{
	ECS_DECLARE_COMPONENT
public:

	vec3 origin;

	float radius1;
	float radius2;

	float timeOffset1;
	float timeOffset2;

	float timeMult1;
	float timeMult2;

	float angle1;
	float angle2;
};


//---------------------------------
// SwirlyLightComponentDesc
//
// Serialize / Deserialize
//
class SwirlyLightComponentDesc final : public fw::ComponentDescriptor<SwirlyLightComponent, true>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<SwirlyLightComponent, true>)
	DECLARE_FORCED_LINKING()

	// construct destruct
	//--------------------
public:
	SwirlyLightComponentDesc() : ComponentDescriptor<SwirlyLightComponent, true>() {}

	// ComponentDescriptor interface
	//-------------------------------
	SwirlyLightComponent* MakeData() override;
	void OnScenePostLoad(fw::EcsController& ecs, fw::T_EntityId const id, SwirlyLightComponent& comp);
	
	// Data
	///////

	float minRadius = 0.2f;
	float maxRadius = 1.f;

	float minTimeMult = -1.f;
	float maxTimeMult = 1.f;

	float bounds = 1.f;
};


} // namespace demo

