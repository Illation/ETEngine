#pragma once
#include "AbstractComponent.h"

#include <EtRendering/PlanetTech/Planet.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>


//---------------------------------
// PlanetComponent
//
// Adds planet geometry to an entity
//
class PlanetComponent : public AbstractComponent
{
	// construct destruct
	//--------------------
public:
	PlanetComponent(render::PlanetParams const& params);
	virtual ~PlanetComponent() = default;

private:
	PlanetComponent(const PlanetComponent& yRef);
	PlanetComponent& operator=(const PlanetComponent& yRef);

	// accessors
	//-----------
public:
	float GetRadius() const { return m_Params.radius; }
	float GetMaxHeight() const { return m_Params.height; }

	// component interface
	//---------------------
protected:
	void Init() override;
	void Deinit() override;
	void Update() override;

	// Data
	///////
private:

	render::PlanetParams m_Params;

	bool m_Rotate = false;
	core::T_SlotId m_PlanetId = core::INVALID_SLOT_ID;
};


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


} // namespace fw
