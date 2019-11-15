#pragma once
#include <Engine/SceneGraph/Entity.h>

#include <Engine/PlanetTech/Planet.h>


//-----------------------------
// PlanetTemplate
//
// Entity that can manage a renderable Planet
//
class PlanetTemplate : public Entity
{
public:
	Planet() = default;
	virtual ~Planet() = default;

	void SetSunlight(core::T_SlotId const lightId);

protected:
	void Initialize() override;
	void Update() override;

	void Draw() override {}
	void DrawForward() override {}

	virtual void LoadPlanet() = 0;

protected:

	PlanetParams m_Params;

private:
	bool m_Rotate = false;
	core::T_SlotId m_PlanetId;
};

