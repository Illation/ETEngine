#pragma once
#include "..\Engine\PlanetTech\Planet.hpp"

class Earth:public Planet
{
public:
	Earth();
	~Earth();

protected:
	void LoadPlanet();
};
