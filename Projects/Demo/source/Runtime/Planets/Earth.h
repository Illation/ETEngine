#pragma once
#include <Engine/PlanetTech/Planet.h>

class Earth:public Planet
{
public:
	Earth();
	~Earth();

protected:
	void LoadPlanet();
};
