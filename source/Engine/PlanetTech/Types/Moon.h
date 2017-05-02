#pragma once
#include "../Planet.h"
class Moon:public Planet
{
public:
	Moon();
	~Moon();

protected:
	void LoadPlanet();

	Atmosphere* m_pAtmosphere;
};
