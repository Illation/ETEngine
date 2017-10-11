#include "stdafx.hpp"
#include "Moon.hpp"

#include "../../Graphics/TextureData.hpp"

#include "../Atmosphere.hpp"

Moon::Moon():Planet()
{
	m_Radius = 1737.1f;
	m_MaxHeight = 10.7f;
}
Moon::~Moon()
{
	delete m_pAtmosphere;
	m_pAtmosphere = nullptr;
}

void Moon::LoadPlanet()
{
	m_pDiffuse = CONTENT::Load<TextureData>("Resources/Textures/PlanetTextures/moon8k.jpg");
	m_pHeight = CONTENT::Load<TextureData>("Resources/Textures/PlanetTextures/MoonHeight.jpg");

	m_pAtmosphere = new Atmosphere();
	m_pAtmosphere->Initialize();
	SetAtmosphere(m_pAtmosphere, 100);
}