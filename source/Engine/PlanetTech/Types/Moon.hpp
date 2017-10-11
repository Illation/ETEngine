#pragma once

#include "../Planet.hpp"

class Moon:public Planet
{
public:
	Moon();
	~Moon();

protected:
	void LoadPlanet();

	Atmosphere* m_pAtmosphere;
};
