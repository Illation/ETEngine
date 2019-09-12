#pragma once

#include <Engine/PlanetTech/Planet.h>


class Moon:public Planet
{
public:
	Moon();
	~Moon();

protected:
	void LoadPlanet();

	Atmosphere* m_pAtmosphere;
};
