#pragma once
#include <EtFramework/SceneGraph/Entity.h>

#include <EtRendering/PlanetTech/Planet.h>


//-----------------------------
// PlanetTemplate
//
// Entity that can manage a renderable Planet
//
class PlanetTemplate : public Entity
{
public:
	PlanetTemplate() = default;
	virtual ~PlanetTemplate();

	void SetSunlight(core::T_SlotId const lightId);

	float GetRadius() const { return m_Params.radius; }
	float GetMaxHeight() const { return m_Params.height; }
	float GetAtmosphereHeight() const { return m_AtmosphereHeight; }

protected:
	void OnPostComponentInit() override;
	void Update() override;

protected:

	render::PlanetParams m_Params;

	T_Hash m_AtmosphereAssetId = 0u;
	float m_AtmosphereHeight = 0.f;

private:
	bool m_Rotate = false;
	core::T_SlotId m_PlanetId = core::INVALID_SLOT_ID;

	core::T_SlotId m_AtmoId = core::INVALID_SLOT_ID;
};

