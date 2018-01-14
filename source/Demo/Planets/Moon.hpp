#pragma once
#include "..\Engine\PlanetTech\Planet.hpp"

class Moon:public Planet
{
public:
	Moon();
	~Moon();

protected:
	void LoadPlanet();

	Atmosphere* m_pAtmosphere;
};
