#pragma once
#include "../Planet.h"
class Earth:public Planet
{
public:
	Earth();
	~Earth();

protected:
	void LoadPlanet();
};
