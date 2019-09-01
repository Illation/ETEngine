#include "stdafx.h"
#include "Moon.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/Graphics/TextureData.h>
#include <Engine/PlanetTech/Atmosphere.h>


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
	m_pDiffuse = ResourceManager::GetInstance()->GetAssetData<TextureData>("moon8k.jpg"_hash);
	m_pHeight = ResourceManager::GetInstance()->GetAssetData<TextureData>("MoonHeight.jpg"_hash);

	m_pAtmosphere = new Atmosphere("atmo_earth.json"_hash);
	m_pAtmosphere->Initialize();
	SetAtmosphere(m_pAtmosphere, 100);
}