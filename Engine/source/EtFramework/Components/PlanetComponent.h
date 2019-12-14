#pragma once
#include "AbstractComponent.h"

#include <EtRendering/PlanetTech/Planet.h>


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
