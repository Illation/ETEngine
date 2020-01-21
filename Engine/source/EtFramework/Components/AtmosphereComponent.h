#pragma once
#include <EtRendering/PlanetTech/Planet.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>
#include <EtFramework/SceneGraph/EntityLink.h>


namespace et {
namespace fw {


//---------------------------------
// AtmosphereComponent
//
// Adds atmospheric scattering to an entity
//
class AtmosphereComponent final
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	friend class AtmosphereInit;

	// construct destruct
	//--------------------
public:
	AtmosphereComponent(T_Hash const assetId, float const height, float const groundHeight);
	~AtmosphereComponent() = default;

	// functionality
	//---------------
public:
	void SetSunlight(core::T_SlotId const lightId);

	// accessors
	//-----------
	float GetAtmosphereHeight() const { return m_Height; }

	// Data
	///////
private:

	T_Hash m_AssetId = 0u;
	float m_Height = 0.f;
	float m_GroundHeight = 0.f;

	core::T_SlotId m_RenderId = core::INVALID_SLOT_ID;
};


//---------------------------------
// AtmosphereComponentDesc
//
// Descriptor for serialization and deserialization of atmosphere components
//
class AtmosphereComponentDesc final : public ComponentDescriptor<AtmosphereComponent, true>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<AtmosphereComponent, true>)

	// construct destruct
	//--------------------
public:
	AtmosphereComponentDesc() : ComponentDescriptor<AtmosphereComponent, true>() {}
	~AtmosphereComponentDesc() = default;

	// ComponentDescriptor interface
	//-------------------------------
	AtmosphereComponent* MakeData() override;
	void OnScenePostLoad(EcsController& ecs, T_EntityId const id, AtmosphereComponent& comp);

	// Data
	///////

	std::string asset;
	float height;
	float groundRadius;
	EntityLink sun;
};


} // namespace fw
} // namespace et
