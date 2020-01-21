#pragma once
#include <EtRendering/PlanetTech/Planet.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>
#include <EtFramework/SceneGraph/EntityLink.h>


namespace et {
namespace fw {


//---------------------------------
// PlanetComponent
//
// Adds planet geometry to an entity
//
class PlanetComponent final
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	friend class PlanetInit;


	// construct destruct
	//--------------------
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
// PlanetComponentDesc
//
// Descriptor for serialization and deserialization of planet components
//
class PlanetComponentDesc final : public ComponentDescriptor<PlanetComponent>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<PlanetComponent>)

	// construct destruct
	//--------------------
public:
	PlanetComponentDesc() : ComponentDescriptor<PlanetComponent>() {}
	~PlanetComponentDesc() = default;

	// ComponentDescriptor interface
	//-------------------------------
	PlanetComponent* MakeData() override;

	// Data
	///////

	float radius = 0.f;
	float height = 0.f;

	std::string texDiffuse;
	std::string texDetail1;
	std::string texDetail2;

	std::string texHeight;
	std::string texHeightDetail;
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
