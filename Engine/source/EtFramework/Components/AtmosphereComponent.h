#pragma once
#include "AbstractComponent.h"

#include <EtRendering/PlanetTech/Planet.h>


//---------------------------------
// AtmosphereComponent
//
// Adds atmospheric scattering to an entity
//
class AtmosphereComponent : public AbstractComponent
{
	// construct destruct
	//--------------------
public:
	AtmosphereComponent(T_Hash const assetId, float const height, float const groundHeight);
	virtual ~AtmosphereComponent() = default;

private:
	AtmosphereComponent(const AtmosphereComponent& yRef);
	AtmosphereComponent& operator=(const AtmosphereComponent& yRef);

	// functionality
	//---------------
public:
	void SetSunlight(core::T_SlotId const lightId);

	// accessors
	//-----------
	float GetAtmosphereHeight() const { return m_Height; }

	// component interface
	//---------------------
protected:
	void Init() override;
	void Deinit() override;
	void Update() override {}

	// Data
	///////
private:

	T_Hash m_AssetId = 0u;
	float m_Height = 0.f;
	float m_GroundHeight = 0.f;

	core::T_SlotId m_RenderId = core::INVALID_SLOT_ID;
};
