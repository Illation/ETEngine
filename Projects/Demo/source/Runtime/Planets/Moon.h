#pragma once
#include <Engine/Prefabs/PlanetTemplate.h>


class Moon : public PlanetTemplate
{
public:
	Moon() = default;
	~Moon() = default;

protected:
	void LoadPlanet() override;
};
