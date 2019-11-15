#pragma once
#include <Engine/Prefabs/PlanetTemplate.h>

class Earth : public PlanetTemplate
{
public:
	Earth() = default;
	~Earth() = default;

protected:
	void LoadPlanet();
};
