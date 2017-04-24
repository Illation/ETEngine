#include "stdafx.hpp"
#include "Moon.h"

#include "../../Graphics/TextureData.hpp"

Moon::Moon():Planet()
{
	m_Radius = 1737.1f;
	m_MaxHeight = 10.7f;
}
Moon::~Moon()
{
}

void Moon::LoadPlanet()
{
	m_pDiffuse = CONTENT::Load<TextureData>("Resources/Textures/PlanetTextures/moon8k.jpg");
	m_pHeight = CONTENT::Load<TextureData>("Resources/Textures/PlanetTextures/MoonHeight.jpg");
}