#pragma once
#include <EtRendering/PlanetTech/Planet.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>
#include <EtFramework/SceneGraph/EntityLink.h>


namespace et {
	REGISTRATION_NS(fw);
}


namespace et {
namespace fw {


//---------------------------------
// PlanetComponent
//
// Adds planet geometry to an entity
//
class PlanetComponent final : public SimpleComponentDescriptor
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	RTTR_ENABLE(SimpleComponentDescriptor) // for serialization
	REGISTRATION_FRIEND_NS(fw)

	friend class PlanetInit;


	// construct destruct
	//--------------------
	PlanetComponent() = default;
public:
	PlanetComponent(render::PlanetParams const& params);
	~PlanetComponent() = default;

	// accessors
	//-----------
public:
	float GetRadius() const { return m_Params.radius; }
	float GetMaxHeight() const { return m_Params.height; }

	// Data
	///////
private:

	render::PlanetParams m_Params;
	core::T_SlotId m_PlanetId = core::INVALID_SLOT_ID;
};

//---------------------------------
// PlanetCameraLinkComponent
//
// Tags a camera so it uses the planet terrain to adjust its clipping planes
//
struct PlanetCameraLinkComponent final
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	// construct destruct
	//--------------------
public:
	PlanetCameraLinkComponent(T_EntityId const p) : planet(p) {}

	T_EntityId const planet;
};

//---------------------------------
// PlanetCameraLinkComponentDesc
//
// Descriptor for serialization and deserialization of planet camera link components
//
class PlanetCameraLinkComponentDesc final : public ComponentDescriptor<PlanetCameraLinkComponent>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<PlanetCameraLinkComponent>)
	DECLARE_FORCED_LINKING()

	// construct destruct
	//--------------------
public:
	PlanetCameraLinkComponentDesc() : ComponentDescriptor<PlanetCameraLinkComponent>() {}
	~PlanetCameraLinkComponentDesc() = default;

	// ComponentDescriptor interface
	//-------------------------------
	PlanetCameraLinkComponent* MakeData() override;

	// Data
	///////

	EntityLink planet;
};


} // namespace fw
} // namespace et
