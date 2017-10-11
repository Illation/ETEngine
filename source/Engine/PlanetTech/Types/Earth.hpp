#pragma once

#include "../Planet.hpp"

class Earth:public Planet
{
public:
	Earth();
	~Earth();

protected:
	void LoadPlanet();
};
